#pragma once
#include <Core/Core.hpp>
#include <Core/FixedString.hpp>

namespace quinte
{
    namespace audio
    {
        enum class Format : uint32_t
        {
            None = 0,
            Int8 = 1 << 0,
            Int16 = 1 << 1,
            Int24 = 1 << 2,
            Int32 = 1 << 3,
            Float32 = 1 << 4,
            Float64 = 1 << 5,

            Max = Float64,
        };

        QU_ENUM_BIT_OPERATORS(Format);


        // clang-format off
        template<Format TFormat> struct FormatTypeHelper { };
        template<> struct FormatTypeHelper<Format::Int8> { using Type = int8_t; };
        template<> struct FormatTypeHelper<Format::Int16> { using Type = int16_t; };
        template<> struct FormatTypeHelper<Format::Int24> { using Type = int24_t; };
        template<> struct FormatTypeHelper<Format::Int32> { using Type = int32_t; };
        template<> struct FormatTypeHelper<Format::Float32> { using Type = float; };
        template<> struct FormatTypeHelper<Format::Float64> { using Type = double; };
        // clang-format on

        template<Format TFormat>
        using FormatType = FormatTypeHelper<TFormat>::Type;


        inline constexpr uint32_t GetFormatByteSize(Format format)
        {
            switch (format)
            {
            default:
                return 0;
            case Format::Int8:
                return sizeof(FormatType<Format::Int8>);
            case Format::Int16:
                return sizeof(FormatType<Format::Int16>);
            case Format::Int24:
                return sizeof(FormatType<Format::Int24>);
            case Format::Int32:
                return sizeof(FormatType<Format::Int32>);
            case Format::Float32:
                return sizeof(FormatType<Format::Float32>);
            case Format::Float64:
                return sizeof(FormatType<Format::Float64>);
            }
        }


        inline constexpr bool IsFloatFormat(Format format)
        {
            return format == Format::Float64 || format == Format::Float32;
        }


        enum class SampleRate : uint32_t
        {
            Value_4000 = 1 << 0,
            Value_5512 = 1 << 1,
            Value_8000 = 1 << 2,
            Value_9600 = 1 << 3,
            Value_11025 = 1 << 4,
            Value_16000 = 1 << 5,
            Value_22050 = 1 << 6,
            Value_32000 = 1 << 7,
            Value_44100 = 1 << 8,
            Value_48000 = 1 << 9,
            Value_88200 = 1 << 10,
            Value_96000 = 1 << 11,
            Value_176400 = 1 << 12,
            Value_192000 = 1 << 13,

            Value_None = 0,
            Value_All = (1 << 14) - 1,
        };

        QU_ENUM_BIT_OPERATORS(SampleRate);

        inline constexpr uint32_t ConvertSampleRate(SampleRate sampleRate) noexcept
        {
            switch (sampleRate)
            {
                // clang-format off
                case SampleRate::Value_4000:   return 4000;
                case SampleRate::Value_5512:   return 5512;
                case SampleRate::Value_8000:   return 8000;
                case SampleRate::Value_9600:   return 9600;
                case SampleRate::Value_11025:  return 11025;
                case SampleRate::Value_16000:  return 16000;
                case SampleRate::Value_22050:  return 22050;
                case SampleRate::Value_32000:  return 32000;
                case SampleRate::Value_44100:  return 44100;
                case SampleRate::Value_48000:  return 48000;
                case SampleRate::Value_88200:  return 88200;
                case SampleRate::Value_96000:  return 96000;
                case SampleRate::Value_176400: return 176400;
                case SampleRate::Value_192000: return 192000;
                default:                       return 0;
                // clang-format on
            }
        }

        inline constexpr SampleRate ConvertSampleRate(uint32_t sampleRate) noexcept
        {
            switch (sampleRate)
            {
                // clang-format off
                case 4000:   return SampleRate::Value_4000;
                case 5512:   return SampleRate::Value_5512;
                case 8000:   return SampleRate::Value_8000;
                case 9600:   return SampleRate::Value_9600;
                case 11025:  return SampleRate::Value_11025;
                case 16000:  return SampleRate::Value_16000;
                case 22050:  return SampleRate::Value_22050;
                case 32000:  return SampleRate::Value_32000;
                case 44100:  return SampleRate::Value_44100;
                case 48000:  return SampleRate::Value_48000;
                case 88200:  return SampleRate::Value_88200;
                case 96000:  return SampleRate::Value_96000;
                case 176400: return SampleRate::Value_176400;
                case 192000: return SampleRate::Value_192000;
                default:     return SampleRate::Value_None;
                // clang-format on
            }
        }


        enum class ResultCode : int32_t
        {
            Success = 0,

            FailUnknown = -1,
            FailUninitialized = -2,
            FailUnsupportedAPI = -3,
            FailDeviceNotFound = -4,
            FailDeviceModeNotSupported = -5,
            FailStreamNotRunning = -6,
        };


        inline constexpr bool Failed(ResultCode error)
        {
            return enum_cast(error) < 0;
        }


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
    };


    class AudioEngine final
    {
        memory::unique_ptr<IAudioAPI> m_Impl;

    public:
        inline IAudioAPI* GetAPI() const
        {
            return m_Impl.get();
        }

        audio::ResultCode InitializeAPI(audio::APIKind apiKind);
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
