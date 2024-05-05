#pragma once
#include <Core/Core.hpp>
#include <Core/Interface.hpp>
#include <Core/EventBus.hpp>
#include <Audio/AudioEngineEvents.hpp>

namespace quinte
{
    class PortManager;


    class Session final
        : public Interface<Session>::Registrar
        , public EventBus<AudioEngineEvents>::Handler
    {
        memory::unique_ptr<PortManager> m_pPortManager;

        void OnAudioStreamStarted() override;
        void OnAudioStreamStopped() override;

    public:
        ~Session();
    };
} // namespace quinte
