#pragma once
#include <Audio/Base.hpp>
#include <Audio/Ports/AudioPort.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Tracks/AudioClip.hpp>
#include <Audio/Tracks/Fader.hpp>
#include <Audio/Tracks/Playlist.hpp>
#include <Core/String.hpp>

namespace quinte
{
    namespace audio
    {
        enum class TrackFlags : uint8_t
        {
            None = 0,

            Master = 1 << 0,
            Monitored = 1 << 1,
            RecordArmed = 1 << 2,
        };

        QU_ENUM_BIT_OPERATORS(TrackFlags);
    } // namespace audio


    class Track final : public memory::RefCountedObjectBase
    {
        using PortContainer = SmallVector<Rc<Port>, 2>;

        PortContainer m_InputPorts;
        PortContainer m_OutputPorts;
        Playlist m_Playlist;
        [[maybe_unused]] audio::DataType m_InputDataType;
        [[maybe_unused]] audio::DataType m_OutputDataType;
        std::atomic<audio::TrackFlags> m_Flags = audio::TrackFlags::None;
        Fader m_Fader;
        String m_Name;

        inline static void ShrinkPorts(PortContainer& ports)
        {
            while (!ports.empty() && !ports.back())
                ports.pop_back();
            if (ports.size() <= ports.inline_capacity())
                ports.shrink_to_fit();
        }

        inline static void SetPort(PortContainer& ports, uint32_t channelIndex, Port* pPort)
        {
            ports.resize(channelIndex + 1);
            ports[channelIndex] = pPort;
            ShrinkPorts(ports);
        }

        inline Port* EnsurePortExists(uint32_t channelIndex, audio::DataDirection dir)
        {
            PortContainer& ports = dir == audio::DataDirection::Input ? m_InputPorts : m_OutputPorts;
            ports.resize(channelIndex + 1);
            if (ports[channelIndex])
                return ports[channelIndex].Get();

            QU_AssertDebugMsg(m_InputDataType == audio::DataType::Audio, "not implemented");
            QU_AssertDebugMsg(m_OutputDataType == audio::DataType::Audio, "not implemented");

            const audio::PortDesc portDesc{ .Kind = audio::PortKind::Track, .Direction = dir };
            ports[channelIndex] = Interface<PortManager>::Get()->NewAudioPort(portDesc);
            ports[channelIndex]->AllocateBuffer();
            return ports[channelIndex].Get();
        }

    public:
        struct MasterConstruct final
        {
        };


        inline Track(MasterConstruct, const StereoPorts& inputPorts, const StereoPorts& outputPorts)
            : m_InputDataType(audio::DataType::Audio)
            , m_OutputDataType(audio::DataType::Audio)
            , m_Flags(audio::TrackFlags::Master)
            , m_Fader(audio::DataType::Audio)
            , m_Name("Master")
        {
            SetPort(m_InputPorts, 0, inputPorts.Left.Get());
            SetPort(m_InputPorts, 1, inputPorts.Right.Get());
            SetPort(m_OutputPorts, 0, outputPorts.Left.Get());
            SetPort(m_OutputPorts, 1, outputPorts.Right.Get());
        }

        inline Track(audio::DataType inputDataType, audio::DataType outputDataType)
            : m_InputDataType(inputDataType)
            , m_OutputDataType(outputDataType)
            , m_Fader(outputDataType)
        {
            PortManager* pPortManager = Interface<PortManager>::Get();

            m_InputPorts
                .emplace_back(pPortManager->NewAudioPort(audio::PortDesc{
                    .Kind = audio::PortKind::Track,
                    .Direction = audio::DataDirection::Input,
                    .Flags = audio::PortFlags::StereoLeft,
                }))
                ->AllocateBuffer();
            m_InputPorts
                .emplace_back(pPortManager->NewAudioPort(audio::PortDesc{
                    .Kind = audio::PortKind::Track,
                    .Direction = audio::DataDirection::Input,
                    .Flags = audio::PortFlags::StereoRight,
                }))
                ->AllocateBuffer();
            m_OutputPorts
                .emplace_back(pPortManager->NewAudioPort(audio::PortDesc{
                    .Kind = audio::PortKind::Track,
                    .Direction = audio::DataDirection::Output,
                    .Flags = audio::PortFlags::StereoLeft,
                }))
                ->AllocateBuffer();
            m_OutputPorts
                .emplace_back(pPortManager->NewAudioPort(audio::PortDesc{
                    .Kind = audio::PortKind::Track,
                    .Direction = audio::DataDirection::Output,
                    .Flags = audio::PortFlags::StereoRight,
                }))
                ->AllocateBuffer();
        }

        inline void AddSource(uint32_t channelIndex, Port* pPort)
        {
            Port* pDest = EnsurePortExists(channelIndex, audio::DataDirection::Input);
            Interface<PortManager>::Get()->ConnectPorts(pPort, pDest);
        }

        [[nodiscard]] inline std::span<const Rc<Port>> GetInputPorts() const
        {
            return m_InputPorts;
        }

        [[nodiscard]] inline std::span<const Rc<Port>> GetOutputPorts() const
        {
            return m_OutputPorts;
        }

        [[nodiscard]] inline Playlist& GetPlaylist()
        {
            return m_Playlist;
        }

        [[nodiscard]] inline bool IsMaster() const
        {
            return (m_Flags.load(std::memory_order_acquire) & audio::TrackFlags::Master) == audio::TrackFlags::Master;
        }

        [[nodiscard]] inline bool IsMonitored() const
        {
            return (m_Flags.load(std::memory_order_acquire) & audio::TrackFlags::Monitored) == audio::TrackFlags::Monitored;
        }

        [[nodiscard]] inline bool IsRecordArmed() const
        {
            return (m_Flags.load(std::memory_order_acquire) & audio::TrackFlags::RecordArmed) == audio::TrackFlags::RecordArmed;
        }

        [[nodiscard]] inline StringSlice GetName() const
        {
            return m_Name;
        }

        [[nodiscard]] inline Fader* GetFader()
        {
            return &m_Fader;
        }

        inline void SetMonitor(bool value)
        {
            audio::TrackFlags expected = m_Flags.load(std::memory_order_relaxed);
            while (!m_Flags.compare_exchange_weak(expected, SetFlag(expected, audio::TrackFlags::Monitored, value)))
            {
            }
        }

        inline void SetRecordArm(bool value)
        {
            audio::TrackFlags expected = m_Flags.load(std::memory_order_relaxed);
            while (!m_Flags.compare_exchange_weak(expected, SetFlag(expected, audio::TrackFlags::RecordArmed, value)))
            {
            }
        }

        inline void SetName(StringSlice name)
        {
            m_Name = name;
        }
    };
} // namespace quinte
