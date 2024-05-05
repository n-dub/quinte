#pragma once

namespace quinte
{
    struct AudioEngineEvents
    {
        inline virtual void OnAudioStreamStarted() {}
        inline virtual void OnAudioStreamStopped() {}
    };
} // namespace quinte
