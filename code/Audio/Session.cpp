#include <Audio/Session.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Engine.hpp>

namespace quinte
{
    // TODO: we need to invent something better than a plain singleton.
    static Session* g_CurrentSession = nullptr;


    Session::Session()
    {
        m_pAudioEngine = memory::make_unique<AudioEngine>();
    }


    Session::~Session() = default;


    void Session::OnStreamStarted()
    {
        m_pPortManager = memory::make_unique<PortManager>();
    }


    AudioEngine* Session::GetAudioEngine() const
    {
        return m_pAudioEngine.get();
    }


    PortManager* Session::GetPortManager() const
    {
        return m_pPortManager.get();
    }


    Session* Session::LoadEmpty()
    {
        g_CurrentSession = memory::DefaultNew<Session>();
        return g_CurrentSession;
    }


    void Session::Unload()
    {
        memory::DefaultDelete(g_CurrentSession);
        g_CurrentSession = nullptr;
    }


    Session* Session::Get()
    {
        return g_CurrentSession;
    }
} // namespace quinte
