#pragma once
#include <Audio/Tracks/Track.hpp>
#include <Core/String.hpp>
#include <UI/Utils.hpp>

namespace quinte
{
    class TrackMixerView final
    {
    public:
        float MaxVolume = 1.0f;
        Rc<Track> pTrack;

        uint32_t ID = 0;
        uint32_t Color = 0;

        bool Draw();
    };
} // namespace quinte
