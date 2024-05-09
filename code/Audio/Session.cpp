#include <Audio/Engine.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Session.hpp>
#include <Audio/Sources/BufferAudioSource.hpp>
#include <UI/Colors.hpp>
#include <numbers>

namespace quinte
{
    Session::Session() {}


    Session::~Session()
    {
        Interface<AudioEngine>::Get()->Stop();
    }


    void Session::OnAudioStreamStarted()
    {
        m_TrackList = {};

        m_TrackList.AddTrack(Rc<Track>::DefaultNew(audio::DataType::Audio, audio::DataType::Audio), colors::kAzure);
        m_TrackList.AddTrack(Rc<Track>::DefaultNew(audio::DataType::Audio, audio::DataType::Audio), colors::kDarkGreen);
        m_TrackList.AddTrack(Rc<Track>::DefaultNew(audio::DataType::Audio, audio::DataType::Audio), colors::kRebeccaPurple);

        m_TrackList[0].pTrack->SetName("Sine wave");
        m_TrackList[1].pTrack->SetName("Test 2");

        Playlist& testPlaylist = m_TrackList[0].pTrack->GetPlaylist();
        AudioBuffer* pTestBuffer = Rc<AudioBuffer>::DefaultNew(40000);

        const float kFrequency = 440.0f;
        AudioEngine* pEngine = Interface<AudioEngine>::Get();
        const uint32_t sampleRate = pEngine->GetAPI()->GetSampleRate();

        for (uint32_t sampleIndex = 0; sampleIndex < pTestBuffer->GetCapacity(); ++sampleIndex)
        {
            pTestBuffer->Data()[sampleIndex] =
                0.9f * std::sin(2 * std::numbers::pi_v<float> * kFrequency * sampleIndex / sampleRate);
        }

        BufferAudioSource* pTestSource = Rc<BufferAudioSource>::DefaultNew(pTestBuffer);

        AudioClip testClip{ pTestSource, 40000 };
        testPlaylist.InsertClip(std::move(testClip));

        m_pPortManager = memory::make_unique<PortManager>();
    }


    void Session::OnAudioStreamStopped()
    {
        m_pPortManager.reset();
    }
} // namespace quinte
