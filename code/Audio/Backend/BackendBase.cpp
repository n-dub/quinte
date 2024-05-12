#include <Audio/Backend/BackendBase.hpp>

namespace quinte
{
    void AudioBackendBase::ClearStreamData()
    {
        m_StreamData.CallbackInfo = {};
        m_StreamData.State = audio::StreamState::Closed;
        m_StreamData.Mode = BackendStreamMode::None;
        m_StreamData.SampleRate = 0;
        m_StreamData.BufferFrameCount = 0;
        m_StreamData.BufferCount = 0;
        m_StreamData.DeviceID = {};
        m_StreamData.FirstChannelIndex = {};
        m_StreamData.Latency = {};
        m_StreamData.ConversionInfo = {};
        m_StreamData.DeviceBuffer = {};
        m_StreamData.UserBuffer = {};
        m_StreamData.User = {};
        m_StreamData.Device = {};
    }


    void AudioBackendBase::InitializeConversionInfo(BackendStreamMode mode, uint32_t firstChannelIndex)
    {
        const uint32_t modeIndex = enum_cast(mode);
        auto& conversionInfo = m_StreamData.ConversionInfo[modeIndex];

        if (m_StreamData.User.Format == m_StreamData.Device.Format[modeIndex]
            && m_StreamData.User.ChannelCount[0] == m_StreamData.Device.ChannelCount[0]
            && m_StreamData.User.ChannelCount[1] == m_StreamData.Device.ChannelCount[1]
            && (!m_StreamData.Device.Interleaved[modeIndex] || m_StreamData.User.ChannelCount[modeIndex] < 2))
        {
            conversionInfo.IsNeeded = false;
            return;
        }

        conversionInfo.IsNeeded = true;

        auto& user = mode == BackendStreamMode::Input ? conversionInfo.Out : conversionInfo.In;
        auto& device = mode == BackendStreamMode::Output ? conversionInfo.Out : conversionInfo.In;

        user.Format = m_StreamData.User.Format;
        user.Jump = m_StreamData.User.ChannelCount[modeIndex];
        device.Format = m_StreamData.Device.Format[modeIndex];
        device.Jump = m_StreamData.Device.ChannelCount[modeIndex];

        conversionInfo.ChannelCount = std::min(conversionInfo.In.Jump, conversionInfo.Out.Jump);

        if (m_StreamData.Device.Interleaved[modeIndex])
        {
            if (mode == BackendStreamMode::Output)
            {
                for (uint32_t channelIndex = 0; channelIndex < conversionInfo.ChannelCount; ++channelIndex)
                {
                    conversionInfo.In.Offset.push_back(channelIndex * m_StreamData.BufferFrameCount);
                    conversionInfo.Out.Offset.push_back(channelIndex);
                    conversionInfo.In.Jump = 1;
                }
            }
            else
            {
                for (uint32_t channelIndex = 0; channelIndex < conversionInfo.ChannelCount; ++channelIndex)
                {
                    conversionInfo.In.Offset.push_back(channelIndex);
                    conversionInfo.Out.Offset.push_back(channelIndex * m_StreamData.BufferFrameCount);
                    conversionInfo.Out.Jump = 1;
                }
            }
        }
        else
        {
            for (uint32_t channelIndex = 0; channelIndex < conversionInfo.ChannelCount; ++channelIndex)
            {
                conversionInfo.In.Offset.push_back(channelIndex * m_StreamData.BufferFrameCount);
                conversionInfo.Out.Offset.push_back(channelIndex * m_StreamData.BufferFrameCount);
                conversionInfo.In.Jump = 1;
                conversionInfo.Out.Jump = 1;
            }
        }

        if (firstChannelIndex > 0)
        {
            const uint32_t m = m_StreamData.Device.Interleaved[modeIndex] ? 1 : m_StreamData.BufferFrameCount;
            auto& info = mode == BackendStreamMode::Output ? conversionInfo.Out : conversionInfo.In;
            for (uint32_t channelIndex = 0; channelIndex < conversionInfo.ChannelCount; ++channelIndex)
            {
                info.Offset[channelIndex] += firstChannelIndex * m;
            }
        }
    }


    template<class TInFormat, class TOutFormat>
    static bool ConvertImpl(TOutFormat* pOutBuffer, const TInFormat* pInBuffer, uint32_t bufferFrameCount,
                            const BackendConversionInfo& info)
    {
        // why writing crazy generic code when you don't even need it???
        // idk...

        static_assert(std::is_signed_v<TInFormat> || std::is_same_v<TInFormat, Int24>);
        static_assert(std::is_signed_v<TOutFormat> || std::is_same_v<TOutFormat, Int24>);

        constexpr size_t kInFormatBits = sizeof(TInFormat) * 8;
        constexpr size_t kOutFormatBits = sizeof(TOutFormat) * 8;

        for (uint32_t frameIndex = 0; frameIndex < bufferFrameCount; ++frameIndex)
        {
            for (uint32_t channelIndex = 0; channelIndex < info.ChannelCount; ++channelIndex)
            {
                const TInFormat sourceValue = pInBuffer[info.In.Offset[channelIndex]];

                TOutFormat result;
                if constexpr (std::is_same_v<TInFormat, TOutFormat>)
                {
                    result = sourceValue;
                }
                else if constexpr (std::is_floating_point_v<TOutFormat>)
                {
                    if constexpr (std::is_floating_point_v<TInFormat>)
                    {
                        result = static_cast<TOutFormat>(sourceValue);
                    }
                    else
                    {
                        constexpr TOutFormat kMultiplier = static_cast<TOutFormat>(1.0 / (1 << (kInFormatBits - 1)));
                        result = static_cast<TOutFormat>(sourceValue) * kMultiplier;
                    }
                }
                else
                {
                    if constexpr (std::is_floating_point_v<TInFormat>)
                    {
                        constexpr TInFormat kMultiplier = static_cast<TInFormat>(1 << (kOutFormatBits - 1));
                        result = static_cast<TOutFormat>(sourceValue * kMultiplier);
                    }
                    else if constexpr (kInFormatBits < kOutFormatBits)
                    {
                        result = static_cast<TOutFormat>(sourceValue);
                        result <<= (kOutFormatBits - kInFormatBits);
                    }
                    else
                    {
                        result = static_cast<TOutFormat>(sourceValue >> (kInFormatBits - kOutFormatBits));
                    }
                }

                pOutBuffer[info.Out.Offset[channelIndex]] = result;
            }

            pInBuffer += info.In.Jump;
            pOutBuffer += info.Out.Jump;
        }

        return true;
    }


    template<class TInFormat, class TFormatIndex, TFormatIndex... TFormatIndices>
    static bool ConvertInImpl(std::integer_sequence<TFormatIndex, TFormatIndices...>, uint8_t* pOutBuffer,
                              const TInFormat* pInBuffer, uint32_t bufferFrameCount, const BackendConversionInfo& info)
    {
        return ((info.Out.Format == static_cast<audio::Format>(1 << TFormatIndices)
                 && ConvertImpl(reinterpret_cast<audio::FormatType<static_cast<audio::Format>(1 << TFormatIndices)>*>(pOutBuffer),
                                pInBuffer,
                                bufferFrameCount,
                                info)),
                ...);
    }


    template<class TFormatIndex, TFormatIndex... TFormatIndices>
    static bool ConvertBufferImpl(std::integer_sequence<TFormatIndex, TFormatIndices...> sequence, uint8_t* pOutBuffer,
                                  const uint8_t* pInBuffer, uint32_t bufferFrameCount, const BackendConversionInfo& info)
    {
        return ((info.In.Format == static_cast<audio::Format>(1 << TFormatIndices)
                 && ConvertInImpl(
                     sequence,
                     pOutBuffer,
                     reinterpret_cast<const audio::FormatType<static_cast<audio::Format>(1 << TFormatIndices)>*>(pInBuffer),
                     bufferFrameCount,
                     info)),
                ...);
    }


    void AudioBackendBase::ConvertBuffer(uint8_t* pOutBuffer, const uint8_t* pInBuffer, const BackendConversionInfo& info)
    {
        if (pOutBuffer == m_StreamData.DeviceBuffer.get() && m_StreamData.Mode == BackendStreamMode::Duplex
            && info.Out.Jump > info.In.Jump)
        {
            const size_t bytes =
                static_cast<size_t>(m_StreamData.BufferFrameCount) * info.Out.Jump * audio::GetFormatByteSize(info.Out.Format);
            memset(pOutBuffer, 0, bytes);
        }

        constexpr uint32_t kMaxFormatFlagBit = std::countr_zero(enum_cast(audio::Format::Max));

        ConvertBufferImpl(std::make_integer_sequence<uint32_t, kMaxFormatFlagBit + 1>{},
                          pOutBuffer,
                          pInBuffer,
                          m_StreamData.BufferFrameCount,
                          info);
    }


    size_t AudioBackendBase::GetAudioBufferSize() const
    {
        return m_StreamData.BufferFrameCount;
    }


    uint32_t AudioBackendBase::GetSampleRate() const
    {
        return m_StreamData.SampleRate;
    }


    audio::ResultCode AudioBackendBase::OpenStream(audio::StreamOpenInfo& openInfo)
    {
        QU_Assert(m_StreamData.State == audio::StreamState::Closed);
        QU_Assert(audio::GetFormatByteSize(openInfo.Format) > 0);
        QU_Assert(openInfo.pOutputDesc || openInfo.pInputDesc);
        QU_Assert(openInfo.pOutputDesc == nullptr || openInfo.pOutputDesc->ChannelCount > 0);
        QU_Assert(openInfo.pInputDesc == nullptr || openInfo.pInputDesc->ChannelCount > 0);
        QU_Assert(openInfo.pOutputDesc == nullptr || openInfo.pOutputDesc->DeviceID);
        QU_Assert(openInfo.pInputDesc == nullptr || openInfo.pInputDesc->DeviceID);

        QU_AssertMsg(!m_Devices.empty(), "Audio backend API was not initialized by the derived class");

        const auto openImpl =
            [this](audio::StreamOpenInfo& openInfo, const audio::StreamDesc* pStreamDesc, BackendStreamMode mode) {
                const BackendStreamOpenInfo backendOpenInfo{
                    .DeviceID = pStreamDesc->DeviceID,
                    .Mode = mode,
                    .ChannelCount = pStreamDesc->ChannelCount,
                    .FirstChannelIndex = pStreamDesc->FirstChannelIndex,
                    .SampleRate = audio::ConvertSampleRate(openInfo.SampleRate),
                    .Format = openInfo.Format,
                };

                return OpenStreamImpl(backendOpenInfo, openInfo.BufferFrameCount);
            };

        QU_Defer
        {
            if (GetLastResultCode() != audio::ResultCode::Success)
                CloseStream();
        };

        const uint32_t outputChannelCount = openInfo.pOutputDesc ? openInfo.pOutputDesc->ChannelCount : 0;
        if (outputChannelCount)
        {
            const audio::ResultCode result = openImpl(openInfo, openInfo.pOutputDesc, BackendStreamMode::Output);
            if (audio::Failed(result))
                return Ret(result);
        }

        const uint32_t inputChannelCount = openInfo.pInputDesc ? openInfo.pInputDesc->ChannelCount : 0;
        if (inputChannelCount)
        {
            const audio::ResultCode result = openImpl(openInfo, openInfo.pInputDesc, BackendStreamMode::Input);
            if (audio::Failed(result))
                return Ret(result);
        }

        m_StreamData.CallbackInfo.Callback = openInfo.Callback;
        m_StreamData.CallbackInfo.pUserData = openInfo.pUserData;
        m_StreamData.State = audio::StreamState::Stopped;
        return Ret(audio::ResultCode::Success);
    }


    audio::DeviceID AudioBackendBase::GetSelectedInputDevice() const
    {
        return m_StreamData.DeviceID[enum_cast(BackendStreamMode::Input)];
    }


    audio::DeviceID AudioBackendBase::GetSelectedOutputDevice() const
    {
        return m_StreamData.DeviceID[enum_cast(BackendStreamMode::Output)];
    }


    const audio::DeviceDesc& AudioBackendBase::GetDeviceDesc(audio::DeviceID deviceID) const
    {
        return m_Devices[deviceID.Value];
    }


    std::span<const audio::DeviceDesc> AudioBackendBase::GetDevices() const
    {
        return m_Devices;
    }


    audio::StreamState AudioBackendBase::GetState() const
    {
        return m_StreamData.State;
    }
} // namespace quinte
