#pragma once
#include <Application/VulkanApplication.hpp>
#include <Audio/Engine.hpp>

namespace quinte
{
    class Application final : public VulkanApplication
    {
        AudioEngine m_Engine;
        uint32_t m_SelectedDeviceIndex = 0;

    public:
        Application();

        void DrawUI() override;
    };
} // namespace quinte
