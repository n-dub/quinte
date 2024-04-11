#pragma once
#include <Application/VulkanApplication.hpp>

namespace quinte
{
    class Application final : public VulkanApplication
    {
    public:
        inline Application()
            : VulkanApplication("Quinte")
        {
        }

        void DrawUI() override;
    };
} // namespace quinte
