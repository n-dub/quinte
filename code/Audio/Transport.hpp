#pragma once
#include <Audio/Base.hpp>
#include <Core/Interface.hpp>

namespace quinte
{
    namespace audio
    {
        enum class PlayState
        {
            RollRequested,
            Rolling,
            PauseRequested,
            Paused,
        };
    } // namespace audio


    class Transport final : public Interface<Transport>::Registrar
    {
        friend class AudioEngine;

        // data actually used by the engine in the current cycle
        audio::TimePos64 m_Playhead;
        bool m_Recording = false;

        std::atomic<audio::TimePos64> m_PlayheadRequest;
        std::atomic<audio::PlayState> m_PlayState = audio::PlayState::Paused;
        std::atomic<bool> m_RecordingRequested = false;

    public:
        inline void SetPlayhead(uint64_t sampleCount)
        {
            m_PlayheadRequest.store(sampleCount);
        }

        inline void AddToPlayhead(int64_t sampleCount)
        {
            audio::TimePos64 currentPos = m_PlayheadRequest.load();
            audio::TimePos64 newPos;

            do
            {
                newPos = currentPos;
                if (sampleCount < 0 && static_cast<uint64_t>(-sampleCount) > currentPos.GetSampleIndex())
                    newPos = { 0 };
                else
                    newPos.SampleIndex += sampleCount;
            }
            while (!m_PlayheadRequest.compare_exchange_weak(currentPos, newPos));
        }

        inline void RequestPause()
        {
            audio::PlayState expected = audio::PlayState::Rolling;
            while (!m_PlayState.compare_exchange_weak(expected, audio::PlayState::PauseRequested))
            {
                // pause has already been requested
                if (expected == audio::PlayState::Paused || expected == audio::PlayState::PauseRequested)
                    return;

                // roll has been requested but we are not rolling yet, so wait
                expected = audio::PlayState::Rolling;
                _mm_pause();
            }
        }

        inline void RequestRoll()
        {
            audio::PlayState expected = audio::PlayState::Paused;
            while (!m_PlayState.compare_exchange_weak(expected, audio::PlayState::RollRequested))
            {
                if (expected == audio::PlayState::Rolling || expected == audio::PlayState::RollRequested)
                    return;

                expected = audio::PlayState::Paused;
                _mm_pause();
            }
        }

        inline void RequestRecording()
        {
            m_RecordingRequested.store(true);
        }

        [[nodiscard]] inline audio::PlayState GetPlayState() const
        {
            return m_PlayState;
        }

        [[nodiscard]] inline bool IsActuallyRolling() const
        {
            const audio::PlayState playState = m_PlayState;
            return playState == audio::PlayState::Rolling || playState == audio::PlayState::PauseRequested;
        }

        [[nodiscard]] inline bool IsRecordingEnabled() const
        {
            return m_Recording;
        }

        [[nodiscard]] inline audio::TimePos64 GetPlayhead() const
        {
            return m_Playhead;
        }
    };
} // namespace quinte
