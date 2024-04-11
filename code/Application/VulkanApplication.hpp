#include <Application/BaseApplication.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#ifdef _DEBUG
#    define QUINTE_USE_VULKAN_DEBUG_REPORT
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

        void FrameRender(ImDrawData* draw_data);
        void FramePresent();
        void SetupVulkanWindow(VkSurfaceKHR surface, int width, int height);
        VkPhysicalDevice SelectAdapter();

    protected:
        inline VulkanApplication(StringSlice name)
            : BaseApplication(name)
        {
        }

        bool SetupBackend() override;
        void BackendBeginFrame() override;
        void BackendEndFrame() override;

    public:
        ~VulkanApplication() override;
    };
} // namespace quinte
