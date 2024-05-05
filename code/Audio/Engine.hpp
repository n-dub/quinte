#pragma once
#include <Audio/Base.hpp>
#include <Audio/Ports/AudioPort.hpp>
#include <Core/Interface.hpp>

namespace quinte
{
    namespace audio
    {
        enum class APIKind : uint32_t
        {
            Dummy = 0,
            WASAPI = 1 << 0,
            ASIO = 1 << 1,
        };

        QU_ENUM_BIT_OPERATORS(APIKind);


        struct DeviceID : TypedHandle<DeviceID, uint32_t>
        {
        };


        struct DeviceDesc final
        {
            DeviceID ID;
            FixStr256 Name;

            uint32_t OutputChannelCount = 0;
            uint32_t InputChannelCount = 0;
            uint32_t DuplexChannelCount = 0;

            uint32_t CurrentSampleRate = 0;
            uint32_t PreferredSampleRate = 0;

            Format Formats = Format::None;
            SampleRate SampleRates = SampleRate::Value_None;
        };


        enum class StreamStatus : uint32_t
        {
            OK,
            InputOverflow,
            OutputUnderflow,
        };


        enum class StreamState : uint32_t
        {
            Stopped,
            Stopping,
            Running,
            Closed,
        };

        inline constexpr StringSlice ToString(StreamState state)
        {
            switch (state)
            {
            default:
            case StreamState::Stopped:
                return "Stopped";
            case StreamState::Stopping:
                return "Stopping";
            case StreamState::Running:
                return "Running";
            case StreamState::Closed:
                return "Closed";
            }
        }


        enum class CallbackResult : uint32_t
        {
            OK,
            Stop,
            Abort,
        };


        typedef CallbackResult (*Callback)(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount, double streamTime,
                                           StreamStatus status, void* pUserData);


        struct StreamDesc final
        {
            DeviceID DeviceID;
            uint32_t FirstChannelIndex = 0;
            uint32_t ChannelCount = 0;
        };


        struct StreamOpenInfo final
        {
            StreamDesc* pInputDesc = nullptr;
            StreamDesc* pOutputDesc = nullptr;
            Format Format = Format::None;
            SampleRate SampleRate = SampleRate::Value_44100;
            uint32_t BufferFrameCount = 0;
            Callback Callback = nullptr;
            void* pUserData = nullptr;
        };


        struct EngineStartInfo final
        {
            DeviceID InputDevice;
            DeviceID OutputDevice;
            uint32_t BufferSize;
        };
    } // namespace audio


    class IAudioAPI
    {
    public:
        virtual ~IAudioAPI() = default;

        virtual audio::ResultCode UpdateDeviceList() = 0;

        virtual std::span<const audio::DeviceDesc> GetDevices() const = 0;

        virtual audio::DeviceID GetDefautInputDevice() const = 0;
        virtual audio::DeviceID GetDefaultOutputDevice() const = 0;

        virtual audio::ResultCode OpenStream(audio::StreamOpenInfo& openInfo) = 0;
        virtual void CloseStream() = 0;

        virtual audio::ResultCode StartStream() = 0;
        virtual audio::ResultCode StopStream() = 0;
        virtual audio::ResultCode AbortStream() = 0;

        virtual audio::APIKind GetKind() const = 0;
        virtual audio::StreamState GetState() const = 0;
        virtual size_t GetAudioBufferSize() const = 0;
    };


    class AudioEngine final : public Interface<AudioEngine>::Registrar
    {
        memory::unique_ptr<IAudioAPI> m_Impl;
        size_t m_AudioBufferSize = 0;

        std::atomic<bool> m_Running = false;

        StereoPorts m_HardwarePorts;
        StereoPorts m_MonitorPorts;

        static audio::CallbackResult AudioCallbackImpl(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount,
                                                       double streamTime, audio::StreamStatus status, void* pUserData);

        audio::CallbackResult AudioCallback(void* pOutputBuffer, void* pInputBuffer, uint32_t frameCount, double streamTime,
                                            audio::StreamStatus status);

    public:
        inline IAudioAPI* GetAPI() const
        {
            return m_Impl.get();
        }

        inline size_t GetAudioBufferSize() const
        {
            return m_AudioBufferSize;
        }

        audio::ResultCode InitializeAPI(audio::APIKind apiKind);
        audio::ResultCode Start(const audio::EngineStartInfo& startInfo);
        void Stop();
    };
} // namespace quinte


namespace std
{
    template<>
    struct formatter<quinte::audio::APIKind>
    {
        inline constexpr auto parse(format_parse_context& ctx)
        {
            return ctx.begin();
        }

        inline auto format(const quinte::audio::APIKind& apiKind, format_context& ctx)
        {
            using namespace quinte::audio;

            switch (apiKind)
            {
            case APIKind::Dummy:
                return format_to(ctx.out(), "Dummy");
            case APIKind::ASIO:
                return format_to(ctx.out(), "ASIO");
            case APIKind::WASAPI:
                return format_to(ctx.out(), "WASAPI");
            default:
                QU_Assert(0);
                return format_to(ctx.out(), "Unknown");
            }
        }
    };
} // namespace std
