#pragma once
#include <Application/VulkanApplication.hpp>
#include <Audio/Engine.hpp>
#include <Audio/Session.hpp>
#include <Audio/Transport.hpp>
#include <Core/Interface.hpp>
#include <UI/Windows/WorkArea.hpp>

namespace quinte
{
    class Application final
        : public VulkanApplication
        , public Interface<Application>::Registrar
    {
        memory::unique_ptr<AudioEngine> m_pAudioEngine;
        memory::unique_ptr<Transport> m_pTransport;
        memory::unique_ptr<Session> m_pCurrentSession;

        WorkArea m_WorkArea;

    public:
        Application();

        void DrawUI() override;
    };
} // namespace quinte
