#pragma once
#include <Application/VulkanApplication.hpp>
#include <Audio/Engine.hpp>
#include <Audio/Session.hpp>

namespace quinte
{
    class Application final : public VulkanApplication
    {
        memory::unique_ptr<Session> m_pCurrentSession;

        uint32_t m_SelectedInputDeviceIndex = 0;
        uint32_t m_SelectedOutputDeviceIndex = 0;

    public:
        Application();

        void DrawUI() override;
    };
} // namespace quinte
