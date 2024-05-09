#pragma once
#include <Audio/Tracks/Track.hpp>
#include <Core/FixedVector.hpp>

namespace quinte
{
    struct ExecutionGraphNode final
    {
        Rc<Track> Track;
        SmallVector<ExecutionGraphNode*> Outgoing;
    };
} // namespace quinte
