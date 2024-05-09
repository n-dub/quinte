#pragma once
#include <Audio/Tracks/Track.hpp>
#include <UI/Utils.hpp>

namespace quinte
{
    class TrackEditView final
    {
    public:
        Rc<Track> pTrack;

        uint32_t ID = 0;
        uint32_t Color = 0;

        bool Draw(float width);
    };
} // namespace quinte
