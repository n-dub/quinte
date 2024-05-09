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


    void Session::OnAudioStreamStarted()
    {
        m_TrackList = {};

        m_TrackList.AddTrack(Rc<Track>::DefaultNew(audio::DataType::Audio, audio::DataType::Audio), colors::kAzure);
        m_TrackList.AddTrack(Rc<Track>::DefaultNew(audio::DataType::Audio, audio::DataType::Audio), colors::kDarkGreen);
        m_TrackList.AddTrack(Rc<Track>::DefaultNew(audio::DataType::Audio, audio::DataType::Audio), colors::kRebeccaPurple);

        m_TrackList[0].pTrack->SetName("Sine wave");
        m_TrackList[1].pTrack->SetName("Test 2");

        AudioEngine* pEngine = Interface<AudioEngine>::Get();
        const uint32_t sampleRate = pEngine->GetAPI()->GetSampleRate();

        BufferAudioSource* pTestSource1 = Rc<BufferAudioSource>::DefaultNew(GenerateSineWave(1.0f, 440, sampleRate));
        BufferAudioSource* pTestSource2 = Rc<BufferAudioSource>::DefaultNew(GenerateSineWave(0.5f, 880, sampleRate));

        Playlist& playlist0 = m_TrackList[0].pTrack->GetPlaylist();
        AudioClip testClip0{ "Sine Wave 440Hz", pTestSource1, sampleRate * 0.5f };
        playlist0.InsertClip(std::move(testClip0));
        AudioClip testClip1{ "Sine Wave 440Hz", pTestSource1, sampleRate * 1.5f };
        playlist0.InsertClip(std::move(testClip1));

        Playlist& playlist2 = m_TrackList[2].pTrack->GetPlaylist();
        AudioClip testClip2{ "Sine Wave 880Hz", pTestSource2, sampleRate * 0.7f };
        playlist2.InsertClip(std::move(testClip2));

        m_pPortManager = memory::make_unique<PortManager>();
    }


    void Session::OnAudioStreamStopped()
    {
        m_pPortManager.reset();
    }
} // namespace quinte
