#pragma once
#include <Audio/Tracks/AudioClip.hpp>

namespace quinte
{
    class AudioBufferView;


    class Playlist final
    {
        std::pmr::vector<AudioClip> m_AudioClips;

    public:
        void InsertClip(AudioClip&& clip);
        void Read(AudioBufferView* pDestination, uint64_t dstOffset, audio::TimeRange64 range, uint32_t channelIndex) const;

        inline AudioClip* begin()
        {
            return m_AudioClips.data();
        }

        inline AudioClip* end()
        {
            return m_AudioClips.data() + m_AudioClips.size();
        }

        inline const AudioClip* begin() const
        {
            return m_AudioClips.data();
        }

        inline const AudioClip* end() const
        {
            return m_AudioClips.data() + m_AudioClips.size();
        }
    };
} // namespace quinte
