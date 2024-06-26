﻿#pragma once
#include <Application/BaseApplication.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#ifdef _DEBUG
#    define QU_USE_VULKAN_DEBUG_REPORT
#endif

namespace quinte
{
    class VulkanApplication : public BaseApplication
    {
        VkAllocationCallbacks* m_Allocator = nullptr;
        VkInstance m_Instance = VK_NULL_HANDLE;
        VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
        VkDevice m_Device = VK_NULL_HANDLE;
        uint32_t m_QueueFamily = (uint32_t)-1;
        VkQueue m_Queue = VK_NULL_HANDLE;
        VkDebugReportCallbackEXT m_DebugReport = VK_NULL_HANDLE;
        VkPipelineCache m_PipelineCache = VK_NULL_HANDLE;
        VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;

        ImGui_ImplVulkanH_Window m_MainWindowData;
        int m_MinImageCount = 2;
        bool m_SwapChainRebuild = false;

        bool FrameRender(ImDrawData* draw_data);
        bool FramePresent();
        bool SetupVulkanWindow(VkSurfaceKHR surface, int width, int height);
        VkPhysicalDevice SelectAdapter();

    protected:
        inline VulkanApplication(StringSlice name)
            : BaseApplication(name)
        {
        }

        bool SetupBackend() override;
        bool BackendBeginFrame() override;
        bool BackendEndFrame() override;

    public:
        ~VulkanApplication() override;

        [[nodiscard]] inline int32_t GetWidth() const
        {
            return m_MainWindowData.Width;
        }

        [[nodiscard]] inline int32_t GetHeight() const
        {
            return m_MainWindowData.Height;
        }
    };
} // namespace quinte
