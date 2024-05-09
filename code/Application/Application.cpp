#include <Application/Application.hpp>

namespace quinte
{
    Application::Application()
        : VulkanApplication("Quinte")
    {
        m_pAudioEngine = memory::make_unique<AudioEngine>();
        m_pTransport = memory::make_unique<Transport>();
        m_pCurrentSession = memory::make_unique<Session>();
        Interface<AudioEngine>::Get()->InitializeAPI(audio::APIKind::WASAPI);
    }


    void Application::DrawUI()
    {
        m_WorkArea.Draw();
    }
} // namespace quinte
