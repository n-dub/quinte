#include <Audio/Backend/RingBuffer.hpp>
#include <Audio/Backend/WASAPI.hpp>
#include <Core/TempAllocator.hpp>

#ifndef MF_E_TRANSFORM_NEED_MORE_INPUT
#    define MF_E_TRANSFORM_NEED_MORE_INPUT _HRESULT_TYPEDEF_(0xc00d6d72)
#endif

#ifndef MFSTARTUP_NOSOCKET
#    define MFSTARTUP_NOSOCKET 0x1
#endif

#pragma comment(lib, "ksuser")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "wmcodecdspuuid")


namespace quinte
{
    using windows::CheckHR;


    inline constexpr uint32_t kInput = enum_cast(BackendStreamMode::Input);
    inline constexpr uint32_t kOutput = enum_cast(BackendStreamMode::Output);


    static HRESULT InitializeAudioClient(IAudioClient* pAudioClient, WAVEFORMATEX* pFormat)
    {
        ComPtr<IAudioClient3> pAudioClient3 = nullptr;
        pAudioClient->QueryInterface(__uuidof(IAudioClient3), &pAudioClient3);

        if (pAudioClient3)
        {
            UINT32 minPeriodInFrames;
            const HRESULT hr = pAudioClient3->GetSharedModeEnginePeriod(
                pFormat, &Discard<UINT32>{}, &Discard<UINT32>{}, &minPeriodInFrames, &Discard<UINT32>{});
            if (FAILED(hr))
                return hr;

            return pAudioClient3->InitializeSharedAudioStream(
                AUDCLNT_STREAMFLAGS_EVENTCALLBACK, minPeriodInFrames, pFormat, nullptr);
        }

        return pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_EVENTCALLBACK, 0, 0, pFormat, nullptr);
    }


    static void CopyToMediaBuffer(IMFMediaBuffer* pMediaBuffer, const uint8_t* pSource, size_t size)
    {
        BYTE* pDestination;
        pMediaBuffer->Lock(&pDestination, nullptr, nullptr);
        memcpy(pDestination, pSource, size);
        pMediaBuffer->Unlock();
    }


    static void CopyFromMediaBuffer(IMFMediaBuffer* pMediaBuffer, uint8_t* pDestination, size_t size)
    {
        BYTE* pSource;
        pMediaBuffer->Lock(&pSource, nullptr, nullptr);
        memcpy(pDestination, pSource, size);
        pMediaBuffer->Unlock();
    }


    audio::ResultCode AudioBackendWASAPI::EnumerateDevicesImpl(EDataFlow dataFlow)
    {
        ComPtr<IMMDeviceCollection> pDevices;

        if (!CheckHR(m_DeviceEnumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &pDevices)))
            return audio::ResultCode::FailUnknown;

        UINT deviceCount;
        if (!CheckHR(pDevices->GetCount(&deviceCount)))
            return audio::ResultCode::FailUnknown;

        memory::TempAllocatorScope temp;

        // The list of all currently available devices.
        std::pmr::vector<BackendDeviceID> deviceIDs{ &temp };
        deviceIDs.reserve(deviceCount);

        for (UINT deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex)
        {
            ComPtr<IMMDevice> pDevice;
            if (!CheckHR(pDevices->Item(deviceIndex, &pDevice)))
                continue;

            windows::CoTaskMemPtr<WCHAR> pDeviceStrID;
            if (!CheckHR(pDevice->GetId(pDeviceStrID.GetAddressOf())))
                continue;

            BackendDeviceID& deviceID = deviceIDs.emplace_back();
            windows::WideStringToUTF8(pDeviceStrID.Get(), deviceID);
        }

        for (uint32_t deviceIndex = 0; deviceIndex < m_BackendDevices.size(); ++deviceIndex)
        {
            if (m_BackendDevices[deviceIndex].DataFlow != dataFlow)
                continue;

            // Reset the default state in case it changed.
            m_BackendDevices[deviceIndex].IsDefault = false;
            if (std::find(deviceIDs.begin(), deviceIDs.end(), m_BackendDevices[deviceIndex].ID) != deviceIDs.end())
                continue;

            // Here we remove the devices that are no longer available.
            //
            // Instead of normal std::erase we swap the current element with back() which requires less
            // memory copy operations.
            //  - If the current element is the last one, we don't replace it with itself and just do pop_back()
            //  - Otherwise we need do decrement the index to process the element that had been swapped from the back

            if (deviceIndex < m_BackendDevices.size() - 1)
            {
                m_Devices[deviceIndex] = m_Devices.back();
                m_BackendDevices[deviceIndex] = m_BackendDevices.back();
                --deviceIndex;
            }

            m_Devices.pop_back();
            m_BackendDevices.pop_back();
        }

        for (const BackendDeviceID& deviceID : deviceIDs)
        {
            const auto it =
                std::find_if(m_BackendDevices.begin(), m_BackendDevices.end(), [&deviceID](const DeviceDataFlowInfo& info) {
                    return info.ID == deviceID;
                });

            if (it != m_BackendDevices.end())
                continue;

            audio::DeviceDesc desc{};
            windows::WideString<BackendDeviceID::Cap> wideID{ deviceID };
            if (!GetDeviceInfoImpl(wideID.Data, dataFlow, desc))
                continue;

            desc.ID = audio::DeviceID{ m_CurrentDeviceID++ };
            m_BackendDevices.push_back(DeviceDataFlowInfo{ .ID = deviceID, .DataFlow = dataFlow });
            m_Devices.push_back(desc);
        }

        ComPtr<IMMDevice> pDefaultDevice;
        if (!CheckHR(m_DeviceEnumerator->GetDefaultAudioEndpoint(dataFlow, eConsole, &pDefaultDevice)))
            return audio::ResultCode::Success;

        windows::CoTaskMemPtr<WCHAR> pDeviceStrID;
        if (!FAILED(pDefaultDevice->GetId(pDeviceStrID.GetAddressOf())))
        {
            BackendDeviceID defaultDeviceID;
            windows::WideStringToUTF8(pDeviceStrID.Get(), defaultDeviceID);

            for (uint32_t deviceIndex = 0; deviceIndex < m_BackendDevices.size(); ++deviceIndex)
            {
                if (m_BackendDevices[deviceIndex].ID == defaultDeviceID)
                {
                    m_BackendDevices[deviceIndex].IsDefault = true;
                    break;
                }
            }
        }

        return audio::ResultCode::Success;
    }


    bool AudioBackendWASAPI::GetDeviceInfoImpl(LPCWSTR deviceID, EDataFlow dataFlow, audio::DeviceDesc& deviceDesc)
    {
        ComPtr<IMMDevice> pDevice;
        const HRESULT hrGet = m_DeviceEnumerator->GetDevice(deviceID, &pDevice);
        if (FAILED(hrGet))
            return false;

        ComPtr<IPropertyStore> pProperties;
        const HRESULT hrProp = pDevice->OpenPropertyStore(STGM_READ, &pProperties);
        if (FAILED(hrProp))
            return false;

        PROPVARIANT deviceNameProp;
        PropVariantInit(&deviceNameProp);
        QU_Defer
        {
            PropVariantClear(&deviceNameProp);
        };

        const HRESULT hrPropVal = pProperties->GetValue(PKEY_Device_FriendlyName, &deviceNameProp);
        if (FAILED(hrPropVal) || !deviceNameProp.pwszVal)
            return false;

        windows::WideStringToUTF8(deviceNameProp.pwszVal, deviceDesc.Name);

        ComPtr<IAudioClient> pAudioClient;
        const HRESULT hrClient = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, &pAudioClient);
        if (FAILED(hrClient))
            return false;

        windows::CoTaskMemPtr<WAVEFORMATEX> pFormat;
        const HRESULT hrFormat = pAudioClient->GetMixFormat(pFormat.GetAddressOf());
        if (FAILED(hrFormat))
            return false;

        deviceDesc.SampleRates = audio::SampleRate::Value_All;
        deviceDesc.PreferredSampleRate = pFormat->nSamplesPerSec;

        deviceDesc.DuplexChannelCount = 0;
        if (dataFlow == eCapture)
        {
            deviceDesc.InputChannelCount = pFormat->nChannels;
            deviceDesc.OutputChannelCount = 0;
        }
        else
        {
            QU_Assert(dataFlow == eRender);
            deviceDesc.InputChannelCount = 0;
            deviceDesc.OutputChannelCount = pFormat->nChannels;
        }

        const WAVEFORMATEXTENSIBLE* pFormatExt =
            pFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE ? reinterpret_cast<WAVEFORMATEXTENSIBLE*>(pFormat.Get()) : nullptr;

        if (pFormat->wFormatTag == WAVE_FORMAT_IEEE_FLOAT
            || (pFormatExt && pFormatExt->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
        {
            if (pFormat->wBitsPerSample == 32)
                deviceDesc.Formats = audio::Format::Float32;
            else if (pFormat->wBitsPerSample == 64)
                deviceDesc.Formats = audio::Format::Float64;
        }
        else if (pFormat->wFormatTag == WAVE_FORMAT_PCM || (pFormatExt && pFormatExt->SubFormat == KSDATAFORMAT_SUBTYPE_PCM))
        {
            if (pFormat->wBitsPerSample == 8)
                deviceDesc.Formats = audio::Format::Int8;
            else if (pFormat->wBitsPerSample == 16)
                deviceDesc.Formats = audio::Format::Int16;
            else if (pFormat->wBitsPerSample == 24)
                deviceDesc.Formats = audio::Format::Int24;
            else if (pFormat->wBitsPerSample == 32)
                deviceDesc.Formats = audio::Format::Int32;
        }

        return true;
    }


    audio::ResultCode AudioBackendWASAPI::OpenStreamImpl(const BackendStreamOpenInfo& openInfo, uint32_t& bufferFrameCount)
    {
        const std::unique_lock lock{ m_StreamData.Mutex };

        EDataFlow dataFlow = eRender;
        audio::Format nativeFormat = audio::Format::None;
        BackendDeviceID id;
        for (uint32_t deviceIndex = 0; deviceIndex < m_Devices.size(); ++deviceIndex)
        {
            if (m_Devices[deviceIndex].ID == openInfo.DeviceID)
            {
                id = m_BackendDevices[deviceIndex].ID;
                dataFlow = m_BackendDevices[deviceIndex].DataFlow;
                nativeFormat = m_Devices[deviceIndex].Formats;
                break;
            }
        }

        if (id.Empty())
            return audio::ResultCode::FailDeviceNotFound;

        if (openInfo.Mode != BackendStreamMode::Input && dataFlow == eCapture)
            return audio::ResultCode::FailDeviceModeNotSupported;

        windows::WideString<BackendDeviceID::Cap> wideID{ id };
        ComPtr<IMMDevice> pDevice;
        if (!CheckHR(m_DeviceEnumerator->GetDevice(wideID.Data, &pDevice)))
            return audio::ResultCode::FailUnknown;

        const auto initClient = [this, mode = openInfo.Mode, pDevice = pDevice.Get()](ClientHandle& clientHandle) {
            QU_Assert(clientHandle.AudioClient == nullptr);
            if (!CheckHR(pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, &clientHandle.AudioClient)))
                return audio::ResultCode::FailUnknown;

            windows::CoTaskMemPtr<WAVEFORMATEX> deviceFormat;
            if (!CheckHR(clientHandle.AudioClient->GetMixFormat(deviceFormat.GetAddressOf())))
                return audio::ResultCode::FailUnknown;

            REFERENCE_TIME latency;
            if (!FAILED(clientHandle.AudioClient->GetStreamLatency(&latency)))
                m_StreamData.Latency[enum_cast(mode)] = static_cast<uint32_t>(latency);

            m_StreamData.Device.ChannelCount[enum_cast(mode)] = deviceFormat->nChannels;
            return audio::ResultCode::Success;
        };

        const audio::ResultCode result = dataFlow == eCapture ? initClient(m_CaptureHandle) : initClient(m_RenderHandle);
        if (audio::Failed(result))
            return result;

        if (m_StreamData.Mode == BackendStreamMode::None)
        {
            m_StreamData.Mode = openInfo.Mode;
        }
        else
        {
            if (m_StreamData.Mode == openInfo.Mode)
                return audio::ResultCode::FailDeviceModeNotSupported;

            m_StreamData.Mode = BackendStreamMode::Duplex;
        }

        const uint32_t modeIndex = enum_cast(openInfo.Mode);
        m_StreamData.DeviceID[modeIndex] = openInfo.DeviceID;
        m_StreamData.SampleRate = openInfo.SampleRate;
        m_StreamData.BufferFrameCount = bufferFrameCount;
        m_StreamData.BufferCount = 1;
        m_StreamData.User.Format = openInfo.Format;
        m_StreamData.User.ChannelCount[modeIndex] = openInfo.ChannelCount;
        m_StreamData.FirstChannelIndex[modeIndex] = openInfo.FirstChannelIndex;
        m_StreamData.Device.Format[modeIndex] = nativeFormat;

        InitializeConversionInfo(openInfo.Mode, openInfo.FirstChannelIndex);

        const uint32_t bufferByteSize = m_StreamData.User.ChannelCount[modeIndex] * m_StreamData.BufferFrameCount
            * audio::GetFormatByteSize(m_StreamData.User.Format);

        m_StreamData.UserBuffer[modeIndex].reset(memory::DefaultNewArray<uint8_t>(bufferByteSize));

        return audio::ResultCode::Success;
    }


    AudioBackendWASAPI::AudioBackendWASAPI()
    {
        CheckHR(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&m_DeviceEnumerator)));
    }


    AudioBackendWASAPI::~AudioBackendWASAPI()
    {
        if (m_StreamData.State.load(std::memory_order_acquire) != audio::StreamState::Closed)
            CloseStream();
    }


    void AudioBackendWASAPI::CloseStream()
    {
        const std::unique_lock lock{ m_StreamData.Mutex };

        const audio::StreamState state = m_StreamData.State.load(std::memory_order_acquire);
        QU_Assert(state != audio::StreamState::Closed);

        if (state != audio::StreamState::Stopped)
            StopStream();

        m_RenderHandle = {};
        m_CaptureHandle = {};

        ClearStreamData();
    }


    audio::APIKind AudioBackendWASAPI::GetKind() const
    {
        return audio::APIKind::WASAPI;
    }


    audio::ResultCode AudioBackendWASAPI::UpdateDeviceList()
    {
        if (!m_DeviceEnumerator)
            return Ret(audio::ResultCode::FailUninitialized);

        QU_Defer
        {
            if (GetLastResultCode() != audio::ResultCode::Success)
            {
                m_Devices.clear();
                m_BackendDevices.clear();
            }
        };

        const audio::ResultCode resultCapture = EnumerateDevicesImpl(eCapture);
        if (audio::Failed(resultCapture))
            return Ret(resultCapture);

        const audio::ResultCode resultRender = EnumerateDevicesImpl(eRender);
        return Ret(resultRender);
    }


    audio::DeviceID AudioBackendWASAPI::GetDefautInputDevice() const
    {
        if (m_Devices.empty())
        {
            Ret(audio::ResultCode::FailUninitialized);
            return audio::DeviceID{};
        }

        Ret(audio::ResultCode::Success);
        for (uint32_t deviceIndex = 0; deviceIndex < m_BackendDevices.size(); ++deviceIndex)
        {
            if (m_BackendDevices[deviceIndex].IsDefault && m_BackendDevices[deviceIndex].DataFlow == eCapture)
                return m_Devices[deviceIndex].ID;
        }

        return m_Devices[0].ID;
    }


    audio::DeviceID AudioBackendWASAPI::GetDefaultOutputDevice() const
    {
        if (m_Devices.empty())
        {
            Ret(audio::ResultCode::FailUninitialized);
            return audio::DeviceID{};
        }

        Ret(audio::ResultCode::Success);
        for (uint32_t deviceIndex = 0; deviceIndex < m_BackendDevices.size(); ++deviceIndex)
        {
            if (m_BackendDevices[deviceIndex].IsDefault && m_BackendDevices[deviceIndex].DataFlow == eRender)
                return m_Devices[deviceIndex].ID;
        }

        return m_Devices[0].ID;
    }


    audio::ResultCode AudioBackendWASAPI::StartStream()
    {
        const std::unique_lock lock{ m_StreamData.Mutex };

        const audio::StreamState currentState = m_StreamData.State.load(std::memory_order_acquire);
        if (currentState == audio::StreamState::Stopping || currentState == audio::StreamState::Closed)
            return Ret(audio::ResultCode::FailUninitialized);

        m_StreamData.State.store(audio::StreamState::Running, std::memory_order_release);

        QU_Assert(!m_StreamData.CallbackInfo.Thread);
        m_StreamData.CallbackInfo.Thread = threading::CreateThread("WASAPI Thread", &RunThread, this);

        return Ret(audio::ResultCode::Success);
    }


    audio::ResultCode AudioBackendWASAPI::StopStream()
    {
        audio::StreamState expectedState = audio::StreamState::Running;
        const bool success = m_StreamData.State.compare_exchange_strong(expectedState, audio::StreamState::Stopping);
        if (!success)
        {
            QU_Assert(false);
            return Ret(audio::ResultCode::FailStreamNotRunning);
        }

        threading::CloseThread(m_StreamData.CallbackInfo.Thread);
        return Ret(audio::ResultCode::Success);
    }


    audio::ResultCode AudioBackendWASAPI::AbortStream()
    {
        return StopStream();
    }


    void AudioBackendWASAPI::RunThread(void* pUserData)
    {
        static_cast<AudioBackendWASAPI*>(pUserData)->RunThreadImpl();
    }


    void AudioBackendWASAPI::RunThreadImpl()
    {
        bool errorFlag = true;
        QU_Defer
        {
            m_StreamData.State.store(audio::StreamState::Stopped, std::memory_order_release);
            if (errorFlag)
                Ret(audio::ResultCode::FailUnknown);
        };

        const windows::CoInitializeScope coInitScope;
        windows::SetThreadProAudio();

        memory::TempAllocatorScope temp;

        uint32_t captureFormatSampleRate = 0;
        memory::unique_temp_ptr<Resampler> pCaptureResampler = nullptr;
        AudioRingBuffer captureBuffer;
        ComPtr<IAudioClient> captureAudioClient = m_CaptureHandle.AudioClient;
        ComPtr<IAudioCaptureClient> captureClient = m_CaptureHandle.CaptureClient;

        if (captureAudioClient)
        {
            windows::CoTaskMemPtr<WAVEFORMATEX> captureFormat;

            if (!CheckHR(captureAudioClient->GetMixFormat(captureFormat.GetAddressOf())))
                return;

            constexpr uint32_t modeIndex = kInput;

            pCaptureResampler.reset(memory::New<Resampler>(&temp,
                                                           m_StreamData.Device.Format[modeIndex],
                                                           m_StreamData.Device.ChannelCount[modeIndex],
                                                           captureFormat->nSamplesPerSec,
                                                           m_StreamData.SampleRate));

            if (!captureClient)
            {
                if (!CheckHR(InitializeAudioClient(captureAudioClient.Get(), captureFormat.Get())))
                    return;

                if (!CheckHR(captureAudioClient->GetService(__uuidof(IAudioCaptureClient), &captureClient)))
                    return;

                auto captureEvent = windows::Event::CreateAutoReset("WASAPI/CaptureEvent");
                QU_Assert(captureEvent);
                if (!captureEvent)
                    return;

                if (!CheckHR(captureAudioClient->SetEventHandle(captureEvent.GetHandle())))
                    return;

                m_CaptureHandle.CaptureClient = captureClient;
                m_CaptureHandle.Event = std::move(captureEvent);

                if (!CheckHR(captureAudioClient->Reset()))
                    return;

                if (!CheckHR(captureAudioClient->Start()))
                    return;
            }

            uint32_t inBufferSize;
            if (!CheckHR(captureAudioClient->GetBufferSize(&inBufferSize)))
                return;

            captureFormatSampleRate = captureFormat->nSamplesPerSec;

            uint32_t outBufferSize = CeilDivide(m_StreamData.BufferFrameCount * captureFormatSampleRate, m_StreamData.SampleRate);

            inBufferSize *= m_StreamData.Device.ChannelCount[modeIndex];
            outBufferSize *= m_StreamData.Device.ChannelCount[modeIndex];

            captureBuffer.Initialize(inBufferSize + outBufferSize,
                                     audio::GetFormatByteSize(m_StreamData.Device.Format[modeIndex]));
        }

        uint32_t renderFormatSampleRate = 0;
        memory::unique_temp_ptr<Resampler> pRenderResampler = nullptr;
        AudioRingBuffer renderBuffer;
        ComPtr<IAudioClient> renderAudioClient = m_RenderHandle.AudioClient;
        ComPtr<IAudioRenderClient> renderClient = m_RenderHandle.RenderClient;

        if (renderAudioClient)
        {
            windows::CoTaskMemPtr<WAVEFORMATEX> renderFormat;

            if (!CheckHR(renderAudioClient->GetMixFormat(renderFormat.GetAddressOf())))
                return;

            constexpr uint32_t modeIndex = kOutput;

            pRenderResampler.reset(memory::New<Resampler>(&temp,
                                                          m_StreamData.Device.Format[modeIndex],
                                                          m_StreamData.Device.ChannelCount[modeIndex],
                                                          renderFormat->nSamplesPerSec,
                                                          m_StreamData.SampleRate));

            if (!renderClient)
            {
                if (!CheckHR(InitializeAudioClient(renderAudioClient.Get(), renderFormat.Get())))
                    return;

                if (!CheckHR(renderAudioClient->GetService(__uuidof(IAudioRenderClient), &renderClient)))
                    return;

                auto renderEvent = windows::Event::CreateAutoReset("WASAPI/RenderEvent");
                QU_Assert(renderEvent);
                if (!renderEvent)
                    return;

                if (!CheckHR(renderAudioClient->SetEventHandle(renderEvent.GetHandle())))
                    return;

                m_RenderHandle.RenderClient = renderClient;
                m_RenderHandle.Event = std::move(renderEvent);

                if (!CheckHR(renderAudioClient->Reset()))
                    return;

                if (!CheckHR(renderAudioClient->Start()))
                    return;
            }

            uint32_t outBufferSize;
            if (!CheckHR(renderAudioClient->GetBufferSize(&outBufferSize)))
                return;

            renderFormatSampleRate = renderFormat->nSamplesPerSec;

            uint32_t inBufferSize = CeilDivide(m_StreamData.BufferFrameCount * renderFormatSampleRate, m_StreamData.SampleRate);

            outBufferSize *= m_StreamData.Device.ChannelCount[modeIndex];
            inBufferSize *= m_StreamData.Device.ChannelCount[modeIndex];

            renderBuffer.Initialize(inBufferSize + outBufferSize,
                                    audio::GetFormatByteSize(m_StreamData.Device.Format[modeIndex]));
        }

        const uint32_t inDeviceChannelCount = m_StreamData.Device.ChannelCount[kInput];
        const uint32_t outDeviceChannelCount = m_StreamData.Device.ChannelCount[kOutput];
        const uint32_t inFormatBytes = audio::GetFormatByteSize(m_StreamData.Device.Format[kInput]);
        const uint32_t outFormatBytes = audio::GetFormatByteSize(m_StreamData.Device.Format[kOutput]);

        uint32_t convBufferSize, deviceBufferSize;
        if (m_StreamData.Mode == BackendStreamMode::Output)
        {
            deviceBufferSize = m_StreamData.BufferFrameCount * outDeviceChannelCount * outFormatBytes;
            convBufferSize = CeilDivide(m_StreamData.BufferFrameCount * renderFormatSampleRate, m_StreamData.SampleRate)
                * outDeviceChannelCount * outFormatBytes;
        }
        else if (m_StreamData.Mode == BackendStreamMode::Input)
        {
            deviceBufferSize = m_StreamData.BufferFrameCount * inDeviceChannelCount * inFormatBytes;
            convBufferSize = CeilDivide(m_StreamData.BufferFrameCount * captureFormatSampleRate, m_StreamData.SampleRate)
                * inDeviceChannelCount * inFormatBytes;
        }
        else
        {
            deviceBufferSize =
                m_StreamData.BufferFrameCount * Max(outDeviceChannelCount * outFormatBytes, inDeviceChannelCount * inFormatBytes);
            convBufferSize = Max(CeilDivide(m_StreamData.BufferFrameCount * renderFormatSampleRate, m_StreamData.SampleRate)
                                     * outDeviceChannelCount * outFormatBytes,
                                 CeilDivide(m_StreamData.BufferFrameCount * captureFormatSampleRate, m_StreamData.SampleRate)
                                     * inDeviceChannelCount * inFormatBytes);
        }

        uint8_t* convBuffer = memory::NewArray<uint8_t>(&temp, convBufferSize);
        m_StreamData.DeviceBuffer.reset(memory::DefaultNewArray<uint8_t>(deviceBufferSize));

        bool callbackPulled = false;
        bool callbackPushed = true;
        bool callbackStopped = false;
        DWORD captureFlags = 0;
        while (m_StreamData.State.load(std::memory_order_acquire) != audio::StreamState::Stopping)
        {
            if (!callbackPulled)
            {
                if (captureAudioClient)
                {
                    uint32_t pullSampleCount =
                        CeilDivide(m_StreamData.BufferFrameCount * captureFormatSampleRate, m_StreamData.SampleRate);
                    convBufferSize = 0;

                    const uint32_t inputChannelCount = m_StreamData.Device.ChannelCount[kInput];
                    const uint32_t inputFormatSize = audio::GetFormatByteSize(m_StreamData.Device.Format[kInput]);
                    while (convBufferSize < m_StreamData.BufferFrameCount)
                    {
                        callbackPulled = captureBuffer.Pull(convBuffer, pullSampleCount * inputChannelCount, inputFormatSize);
                        if (!callbackPulled)
                            break;

                        const uint32_t deviceBufferOffset = convBufferSize * inputChannelCount * inputFormatSize;

                        const uint32_t convSampleCount =
                            pCaptureResampler->Convert(&m_StreamData.DeviceBuffer[deviceBufferOffset],
                                                       convBuffer,
                                                       pullSampleCount,
                                                       convBufferSize == 0 ? std::numeric_limits<uint32_t>::max()
                                                                           : m_StreamData.BufferFrameCount - convBufferSize);

                        convBufferSize += convSampleCount;
                        pullSampleCount = 1;
                    }

                    if (callbackPulled)
                    {
                        if (m_StreamData.ConversionInfo[kInput].IsNeeded)
                        {
                            ConvertBuffer(m_StreamData.UserBuffer[kInput].get(),
                                          m_StreamData.DeviceBuffer.get(),
                                          m_StreamData.ConversionInfo[kInput]);
                        }
                        else
                        {
                            memcpy(m_StreamData.UserBuffer[kInput].get(),
                                   m_StreamData.DeviceBuffer.get(),
                                   static_cast<size_t>(m_StreamData.BufferFrameCount) * m_StreamData.User.ChannelCount[kInput]
                                       * audio::GetFormatByteSize(m_StreamData.User.Format));
                        }
                    }
                }
                else
                {
                    callbackPulled = true;
                }

                if (callbackPulled && !callbackStopped)
                {
                    const audio::CallbackResult callbackResult = m_StreamData.CallbackInfo.Callback(
                        m_StreamData.UserBuffer[kOutput].get(),
                        m_StreamData.UserBuffer[kInput].get(),
                        m_StreamData.BufferFrameCount,
                        GetStreamTime(),
                        captureFlags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY ? audio::StreamStatus::InputOverflow
                                                                              : audio::StreamStatus::OK,
                        m_StreamData.CallbackInfo.pUserData);

                    TickStreamTime();

                    if (callbackResult == audio::CallbackResult::Stop || callbackResult == audio::CallbackResult::Abort)
                    {
                        const threading::Thread stopThread{ "WASAPI/StopThread",
                                                            [](void* pData) {
                                                                static_cast<AudioBackendWASAPI*>(pData)->StopStream();
                                                            },
                                                            this };
                        callbackStopped = true;
                    }
                }
            }

            if (renderAudioClient && callbackPulled)
            {
                if (callbackPushed || convBufferSize == 0)
                {
                    if (m_StreamData.ConversionInfo[kOutput].IsNeeded)
                    {
                        ConvertBuffer(m_StreamData.DeviceBuffer.get(),
                                      m_StreamData.UserBuffer[kOutput].get(),
                                      m_StreamData.ConversionInfo[kOutput]);
                    }
                    else
                    {
                        memcpy(m_StreamData.DeviceBuffer.get(),
                               m_StreamData.UserBuffer[kOutput].get(),
                               static_cast<size_t>(m_StreamData.BufferFrameCount) * m_StreamData.User.ChannelCount[kOutput]
                                   * audio::GetFormatByteSize(m_StreamData.User.Format));
                    }

                    convBufferSize =
                        pRenderResampler->Convert(convBuffer, m_StreamData.DeviceBuffer.get(), m_StreamData.BufferFrameCount);
                }

                callbackPushed = renderBuffer.Push(convBuffer,
                                                   m_StreamData.Device.ChannelCount[kOutput] * convBufferSize,
                                                   audio::GetFormatByteSize(m_StreamData.Device.Format[kOutput]));
            }
            else
            {
                callbackPushed = true;
            }

            if (captureAudioClient)
            {
                if (!callbackPulled)
                    m_CaptureHandle.Event.Wait();

                BYTE* streamBuffer;
                uint32_t bufferFrameCount;
                if (!CheckHR(captureClient->GetBuffer(&streamBuffer, &bufferFrameCount, &captureFlags, nullptr, nullptr)))
                    return;

                if (bufferFrameCount > 0)
                {
                    if (!captureBuffer.Push(streamBuffer,
                                            bufferFrameCount * m_StreamData.Device.ChannelCount[kInput],
                                            audio::GetFormatByteSize(m_StreamData.Device.Format[kInput])))
                    {
                        bufferFrameCount = 0;
                    }
                }

                if (!CheckHR(captureClient->ReleaseBuffer(bufferFrameCount)))
                    return;
            }

            if (renderAudioClient)
            {
                if (callbackPulled && !callbackPushed)
                    m_RenderHandle.Event.Wait();

                uint32_t bufferFrameCount;
                if (!CheckHR(renderAudioClient->GetBufferSize(&bufferFrameCount)))
                    return;

                uint32_t framePaddingCount;
                if (!CheckHR(renderAudioClient->GetCurrentPadding(&framePaddingCount)))
                    return;

                bufferFrameCount -= framePaddingCount;

                if (bufferFrameCount > 0)
                {
                    BYTE* streamBuffer;
                    if (!CheckHR(renderClient->GetBuffer(bufferFrameCount, &streamBuffer)))
                        return;

                    if (!renderBuffer.Pull(streamBuffer,
                                           bufferFrameCount * m_StreamData.Device.ChannelCount[kOutput],
                                           audio::GetFormatByteSize(m_StreamData.Device.Format[kOutput])))
                    {
                        bufferFrameCount = 0;
                    }
                }

                if (!CheckHR(renderClient->ReleaseBuffer(bufferFrameCount, 0)))
                    return;
            }

            if (callbackPushed)
                callbackPulled = false;
        }

        errorFlag = false;
    }


    AudioBackendWASAPI::Resampler::Resampler(audio::Format format, uint32_t channelCount, uint32_t inSampleRate,
                                             uint32_t outSampleRate)
        : Format(format)
        , ChannelCount(channelCount)
        , InSampleRate(inSampleRate)
        , OutSampleRate(outSampleRate)
    {
        CheckHR(MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET));
        CheckHR(
            CoCreateInstance(CLSID_CResamplerMediaObject, nullptr, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pTransformUnk));

        CheckHR(pTransformUnk->QueryInterface(IID_PPV_ARGS(&pTransform)));
        CheckHR(pTransformUnk->QueryInterface(IID_PPV_ARGS(&pResamplerProps)));
        CheckHR(pResamplerProps->SetHalfFilterLength(60));

        const GUID audioFormatGuid = audio::IsFloatFormat(format) ? MFAudioFormat_Float : MFAudioFormat_PCM;

        const uint32_t bytesPerSample = audio::GetFormatByteSize(format);
        const uint32_t bitsPerSample = bytesPerSample * 8;

        CheckHR(MFCreateMediaType(&pMediaType));
        CheckHR(pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
        CheckHR(pMediaType->SetGUID(MF_MT_SUBTYPE, audioFormatGuid));
        CheckHR(pMediaType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, channelCount));
        CheckHR(pMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, inSampleRate));
        CheckHR(pMediaType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, bytesPerSample * channelCount));
        CheckHR(pMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bytesPerSample * channelCount * inSampleRate));
        CheckHR(pMediaType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, bitsPerSample));
        CheckHR(pMediaType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));

        CheckHR(MFCreateMediaType(&pInMediaType));
        CheckHR(pMediaType->CopyAllItems(pInMediaType.Get()));

        CheckHR(pTransform->SetInputType(0, pInMediaType.Get(), 0));

        CheckHR(MFCreateMediaType(&pOutMediaType));
        CheckHR(pMediaType->CopyAllItems(pOutMediaType.Get()));

        CheckHR(pOutMediaType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, outSampleRate));
        CheckHR(pOutMediaType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, bytesPerSample * channelCount * outSampleRate));

        CheckHR(pTransform->SetOutputType(0, pOutMediaType.Get(), 0));

        CheckHR(pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0));
        CheckHR(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0));
        CheckHR(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0));
    }


    AudioBackendWASAPI::Resampler::~Resampler()
    {
        CheckHR(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0));
        CheckHR(pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0));
        MFShutdown();
    }


    uint32_t AudioBackendWASAPI::Resampler::Convert(uint8_t* pOutBuffer, const uint8_t* pInBuffer, uint32_t inSampleCount,
                                                    uint32_t maxOutSampleCount)
    {
        const uint32_t bytesPerSample = audio::GetFormatByteSize(Format);
        const uint32_t inputBufferSize = bytesPerSample * ChannelCount * inSampleCount;

        if (InSampleRate == OutSampleRate)
        {
            memcpy(pOutBuffer, pInBuffer, inputBufferSize);
            return inSampleCount;
        }

        const uint32_t outputBufferSize = maxOutSampleCount == std::numeric_limits<uint32_t>::max()
            ? CeilDivide(inputBufferSize * OutSampleRate, InSampleRate)
            : bytesPerSample * ChannelCount * maxOutSampleCount;

        ComPtr<IMFMediaBuffer> pInMediaBuffer;
        CheckHR(MFCreateMemoryBuffer(inputBufferSize, &pInMediaBuffer));
        CopyToMediaBuffer(pInMediaBuffer.Get(), pInBuffer, inputBufferSize);

        pInMediaBuffer->SetCurrentLength(inputBufferSize);

        ComPtr<IMFSample> pInSample;
        CheckHR(MFCreateSample(&pInSample));
        pInSample->AddBuffer(pInMediaBuffer.Get());

        pTransform->ProcessInput(0, pInSample.Get(), 0);
        pInMediaBuffer.Reset();
        pInSample.Reset();

        MFT_OUTPUT_DATA_BUFFER outDataBuffer = { 0 };
        QU_Defer
        {
            memory::SafeRelease(outDataBuffer.pSample);
            memory::SafeRelease(outDataBuffer.pEvents);
        };

        CheckHR(MFCreateSample(&outDataBuffer.pSample));

        ComPtr<IMFMediaBuffer> pOutMediaBuffer;
        CheckHR(MFCreateMemoryBuffer(outputBufferSize, &pOutMediaBuffer));
        outDataBuffer.pSample->AddBuffer(pOutMediaBuffer.Get());

        if (pTransform->ProcessOutput(0, 1, &outDataBuffer, &Discard<DWORD>{}) == MF_E_TRANSFORM_NEED_MORE_INPUT)
            return 0;

        pOutMediaBuffer.Reset();
        outDataBuffer.pSample->ConvertToContiguousBuffer(&pOutMediaBuffer);

        DWORD currentLength;
        pOutMediaBuffer->GetCurrentLength(&currentLength);
        CopyFromMediaBuffer(pOutMediaBuffer.Get(), pOutBuffer, currentLength);

        return currentLength / bytesPerSample / ChannelCount;
    }
} // namespace quinte
