﻿#include <Audio/Tracks/Playlist.hpp>
#include <algorithm>

namespace quinte
{
    void Playlist::InsertClip(AudioClip&& clip)
    {
        auto iter = std::lower_bound(
            m_AudioClips.begin(), m_AudioClips.end(), clip.GetPosition(), [](const AudioClip& lhs, audio::TimePos64 rhs) {
                return lhs.GetEndPosition() < rhs;
            });

        m_AudioClips.insert(iter, std::move(clip));
    }


    void Playlist::Read(float* pDestination, audio::TimeRange64 range) const
    {
        auto iter = std::lower_bound(
            m_AudioClips.begin(), m_AudioClips.end(), range.StartPos, [](const AudioClip& lhs, audio::TimePos64 rhs) {
                return lhs.GetEndPosition() < rhs;
            });

        while (iter != m_AudioClips.end() && iter->GetPosition() <= range.GetFirstSampleIndex()
               && iter->GetEndPosition() >= range.GetFirstSampleIndex())
        {
            // TODO: zero-out the rest
            [[maybe_unused]] const auto ignore = iter->Read(pDestination, range);
            ++iter;
        }
    }
} // namespace quinte
