#include <Audio/Session.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Engine.hpp>

namespace quinte
{
    Session::Session()
    {
        m_pAudioEngine = memory::make_unique<AudioEngine>();
    }


    Session::~Session() = default;


    void Session::OnStreamStarted()
    {
        m_pPortManager = memory::make_unique<PortManager>();
    }


    void Session::OnStreamStopped()
    {
        m_pPortManager.reset();
    }


    AudioEngine* Session::GetAudioEngine() const
    {
        return m_pAudioEngine.get();
    }


    PortManager* Session::GetPortManager() const
    {
        return m_pPortManager.get();
    }
} // namespace quinte
