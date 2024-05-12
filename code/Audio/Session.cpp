#include <Audio/Engine.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Session.hpp>
#include <Audio/Sources/BufferAudioSource.hpp>
#include <UI/Colors.hpp>
#include <numbers>

namespace quinte
{
    static AudioBuffer* GenerateSineWave(float seconds, uint32_t frequency, uint32_t sampleRate)
    {
        AudioBuffer* pResult = Rc<AudioBuffer>::DefaultNew(static_cast<uint32_t>(sampleRate * seconds));

        for (uint32_t sampleIndex = 0; sampleIndex < pResult->GetCapacity(); ++sampleIndex)
        {
            pResult->Data()[sampleIndex] = 0.9f * std::sin(2 * std::numbers::pi_v<float> * frequency * sampleIndex / sampleRate);
        }

        return pResult;
    }


    Session::Session() {}


    Session::~Session()
    {
        Interface<AudioEngine>::Get()->Stop();
    }


    Track* Session::CreateTrack(audio::DataType inputDataType, audio::DataType outputDataType)
    {
        constexpr uint32_t kTrackColors[] = { colors::kAzure, colors::kDarkGreen, colors::kRebeccaPurple };
        m_TrackList.AddTrack(Rc<Track>::DefaultNew(inputDataType, outputDataType),
                             kTrackColors[m_TrackList.size() % std::size(kTrackColors)]);

        Track* pTrack = m_TrackList.back().pTrack.Get();
        pTrack->AddSource(0, m_pPortManager->GetHardwarePorts()[0].Get());
        m_pMasterTrack->AddSource(0, pTrack->GetOutputPorts()[0].Get());
        m_pMasterTrack->AddSource(1, pTrack->GetOutputPorts()[1].Get());
        return pTrack;
    }


    void Session::OnAudioStreamStarted()
    {
        // Here we hard-code some tracks, clips, etc. for testing purposes.
        // Later this data will be loaded from a session file and edited by users.

        m_pPortManager = memory::make_unique<PortManager>();

        const StereoPorts& monitorPorts = m_pPortManager->GetMonitorPorts();

        const audio::PortDesc masterInPortsDesc{ .Kind = audio::PortKind::Track, .Direction = audio::DataDirection::Input };
        m_MasterInputPorts = StereoPorts::Create(masterInPortsDesc);
        m_MasterInputPorts.Left->AllocateBuffer();
        m_MasterInputPorts.Right->AllocateBuffer();

        const audio::PortDesc masterOutPortsDesc{ .Kind = audio::PortKind::Track, .Direction = audio::DataDirection::Output };
        m_MasterOutputPorts = StereoPorts::Create(masterOutPortsDesc);
        m_MasterOutputPorts.Left->AllocateBuffer();
        m_MasterOutputPorts.Right->AllocateBuffer();

        m_pPortManager->ConnectPorts(m_MasterOutputPorts.Left.Get(), monitorPorts.Left.Get());
        m_pPortManager->ConnectPorts(m_MasterOutputPorts.Right.Get(), monitorPorts.Right.Get());

        m_pMasterTrack = Rc<Track>::DefaultNew(Track::MasterConstruct{}, m_MasterInputPorts, m_MasterOutputPorts);

        m_TrackList = {};
        CreateTrack();
        CreateTrack();
        CreateTrack();

        m_TrackList[0].pTrack->SetName("Sine wave");
        m_TrackList[1].pTrack->SetName("Test 2");

        const uint32_t sampleRate = Interface<AudioEngine>::Get()->GetAPI()->GetSampleRate();

        BufferAudioSource* pTestSource1 = Rc<BufferAudioSource>::DefaultNew(GenerateSineWave(1.0f, 440, sampleRate));
        BufferAudioSource* pTestSource2 = Rc<BufferAudioSource>::DefaultNew(GenerateSineWave(0.5f, 880, sampleRate));

        Playlist& playlist0 = m_TrackList[0].pTrack->GetPlaylist();
        playlist0.InsertClip(AudioClip{ "Sine Wave 440Hz", pTestSource1, sampleRate * 0.5f });
        playlist0.InsertClip(AudioClip{ "Sine Wave 440Hz", pTestSource1, sampleRate * 1.5f });

        Playlist& playlist2 = m_TrackList[2].pTrack->GetPlaylist();
        playlist2.InsertClip(AudioClip{ "Sine Wave 880Hz", pTestSource2, sampleRate * 0.7f });
    }


    void Session::OnAudioStreamStopped()
    {
        m_TrackList = {};
        m_MasterInputPorts = {};
        m_MasterOutputPorts = {};
        m_pMasterTrack.Reset();
        m_pPortManager.reset();
    }
} // namespace quinte
