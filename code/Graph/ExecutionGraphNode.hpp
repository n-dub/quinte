#pragma once
#include <Audio/Tracks/Track.hpp>
#include <Core/FixedVector.hpp>

namespace quinte
{
    struct ExecutionGraphNode final
    {
        Rc<Track> Track;
        SmallVector<ExecutionGraphNode*> Outgoing;
        std::atomic<uint32_t> DependencyCount = 0;
        uint32_t InitialDependencyCount = 0;

        inline bool Trigger()
        {
            return --DependencyCount == 0;
        }
    };
} // namespace quinte
