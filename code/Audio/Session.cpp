#include <Audio/Session.hpp>
#include <Audio/Ports/PortManager.hpp>
#include <Audio/Engine.hpp>

namespace quinte
{
    Session::~Session() = default;


    void Session::OnAudioStreamStarted()
    {
        m_pPortManager = memory::make_unique<PortManager>();
    }


    void Session::OnAudioStreamStopped()
    {
        m_pPortManager.reset();
    }
} // namespace quinte
