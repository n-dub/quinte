#include <Audio/Engine.hpp>
#include <Audio/Backend/WASAPI.hpp>

namespace quinte
{
    audio::CallbackResult AudioEngine::AudioCallbackImpl(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount,
                                                         double streamTime, audio::StreamStatus status, void* pUserData)
    {
        return static_cast<AudioEngine*>(pUserData)->AudioCallback(pOutputBuffer, pInputBuffer, frameCount, streamTime, status);
    }


    audio::CallbackResult AudioEngine::AudioCallback(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount,
                                                     double streamTime, audio::StreamStatus status)
    {
        QU_Unused(streamTime);
        QU_Unused(status);

        if (!m_Running)
        {
            memory::Zero(static_cast<float*>(pOutputBuffer), frameCount * 2);
            return audio::CallbackResult::OK;
        }

        AudioBufferView* pHWL = m_HardwarePorts.Left->GetBufferView();
        pHWL->Read(static_cast<float*>(pInputBuffer), 0, frameCount);

        AudioBufferView* pHWR = m_HardwarePorts.Right->GetBufferView();
        pHWR->Read(static_cast<float*>(pInputBuffer) + frameCount, 0, frameCount);

        AudioBufferView* pML = m_MonitorPorts.Left->GetBufferView();
        pML->Read(pHWL, 0, 0, frameCount);

        AudioBufferView* pMR = m_MonitorPorts.Right->GetBufferView();
        pMR->Read(pHWR, 0, 0, frameCount);

        memory::Copy(static_cast<float*>(pOutputBuffer), pML->Data(), frameCount);
        memory::Copy(static_cast<float*>(pOutputBuffer) + frameCount, pMR->Data(), frameCount);

        return audio::CallbackResult::OK;
    }


    audio::ResultCode AudioEngine::InitializeAPI(audio::APIKind apiKind)
    {
        switch (apiKind)
        {
        case audio::APIKind::WASAPI:
            m_Impl = memory::make_unique<AudioBackendWASAPI>();
            return m_Impl->UpdateDeviceList();
        default:
            return audio::ResultCode::FailUnsupportedAPI;
        }
    }


    audio::ResultCode AudioEngine::Start(const audio::EngineStartInfo& startInfo)
    {
        QU_AssertDebug(m_Impl->GetState() == audio::StreamState::Closed);

        audio::StreamDesc outputDesc{};
        outputDesc.DeviceID = startInfo.OutputDevice;
        outputDesc.ChannelCount = 2;

        audio::StreamDesc inputDesc{};
        inputDesc.DeviceID = startInfo.InputDevice;
        inputDesc.ChannelCount = 2;

        audio::StreamOpenInfo openInfo{};
        openInfo.pOutputDesc = &outputDesc;
        openInfo.pInputDesc = &inputDesc;
        openInfo.Format = audio::Format::Float32;
        openInfo.BufferFrameCount = startInfo.BufferSize;
        openInfo.Callback = &AudioCallbackImpl;
        openInfo.pUserData = this;

        const audio::ResultCode openResult = m_Impl->OpenStream(openInfo);
        if (audio::Failed(openResult))
            return openResult;

        const audio::ResultCode startResult = m_Impl->StartStream();
        if (audio::Failed(startResult))
            return startResult;

        EventBus<AudioEngineEvents>::SendEvent(&AudioEngineEvents::OnAudioStreamStarted);

        const audio::PortDesc hwPortsDesc{ .Kind = audio::PortKind::Hardware, .Direction = audio::DataDirection::Output };
        m_HardwarePorts = StereoPorts::Create(hwPortsDesc);
        m_HardwarePorts.Left->AllocateBuffer();
        m_HardwarePorts.Right->AllocateBuffer();

        const audio::PortDesc monPortsDesc{ .Kind = audio::PortKind::Hardware, .Direction = audio::DataDirection::Input };
        m_MonitorPorts = StereoPorts::Create(monPortsDesc);
        m_MonitorPorts.Left->AllocateBuffer();
        m_MonitorPorts.Right->AllocateBuffer();

        m_Running.store(true);

        return startResult;
    }


    void AudioEngine::Stop()
    {
        m_Running.store(false);
        m_MonitorPorts = {};
        m_HardwarePorts = {};
        m_Impl->CloseStream();
        EventBus<AudioEngineEvents>::SendEvent(&AudioEngineEvents::OnAudioStreamStopped);
    }
} // namespace quinte
