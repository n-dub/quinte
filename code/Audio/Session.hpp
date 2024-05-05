#pragma once
#include <Core/Core.hpp>
#include <Core/Interface.hpp>

namespace quinte
{
    class PortManager;
    class AudioEngine;


    class Session final : public Interface<Session>::Registrar
    {
        memory::unique_ptr<AudioEngine> m_pAudioEngine;
        memory::unique_ptr<PortManager> m_pPortManager;

    public:
        Session();
        ~Session();

        void OnStreamStarted();
        void OnStreamStopped();

        AudioEngine* GetAudioEngine() const;
        PortManager* GetPortManager() const;
    };
} // namespace quinte
