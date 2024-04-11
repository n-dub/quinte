#include <Application/VulkanApplication.hpp>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

namespace quinte
{
#ifdef QUINTE_USE_VULKAN_DEBUG_REPORT
    static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback(VkDebugReportFlagsEXT flags,
                                                                    VkDebugReportObjectTypeEXT objectType, uint64_t object,
                                                                    size_t location, int32_t messageCode,
                                                                    const char* pLayerPrefix, const char* pMessage,
                                                                    void* pUserData)
    {
        QUINTE_Unused(flags);
        QUINTE_Unused(object);
        QUINTE_Unused(location);
        QUINTE_Unused(messageCode);
        QUINTE_Unused(pUserData);
        QUINTE_Unused(pLayerPrefix);
        fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
        return VK_FALSE;
    }
#endif


    static void CheckVulkanResult(VkResult err)
    {
        if (err == 0)
            return;

        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }


    static bool IsExtensionAvailable(std::span<VkExtensionProperties> properties, const char* extension)
    {
        for (const VkExtensionProperties& p : properties)
        {
            if (strcmp(p.extensionName, extension) == 0)
                return true;
        }

        return false;
    }


    void VulkanApplication::FrameRender(ImDrawData* draw_data)
    {
        ImGui_ImplVulkanH_Window* wd = &m_MainWindowData;

        VkResult err;

        VkSemaphore image_acquired_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;
        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        err =
            vkAcquireNextImageKHR(m_Device, wd->Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &wd->FrameIndex);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            m_SwapChainRebuild = true;
            return;
        }
        CheckVulkanResult(err);

        ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
        {
            err = vkWaitForFences(m_Device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);
            CheckVulkanResult(err);

            err = vkResetFences(m_Device, 1, &fd->Fence);
            CheckVulkanResult(err);
        }
        {
            err = vkResetCommandPool(m_Device, fd->CommandPool, 0);
            CheckVulkanResult(err);
            VkCommandBufferBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
            CheckVulkanResult(err);
        }
        {
            VkRenderPassBeginInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            info.renderPass = wd->RenderPass;
            info.framebuffer = fd->Framebuffer;
            info.renderArea.extent.width = wd->Width;
            info.renderArea.extent.height = wd->Height;
            info.clearValueCount = 1;
            info.pClearValues = &wd->ClearValue;
            vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
        }

        ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

        vkCmdEndRenderPass(fd->CommandBuffer);
        {
            VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            VkSubmitInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            info.waitSemaphoreCount = 1;
            info.pWaitSemaphores = &image_acquired_semaphore;
            info.pWaitDstStageMask = &wait_stage;
            info.commandBufferCount = 1;
            info.pCommandBuffers = &fd->CommandBuffer;
            info.signalSemaphoreCount = 1;
            info.pSignalSemaphores = &render_complete_semaphore;

            err = vkEndCommandBuffer(fd->CommandBuffer);
            CheckVulkanResult(err);
            err = vkQueueSubmit(m_Queue, 1, &info, fd->Fence);
            CheckVulkanResult(err);
        }
    }


    void VulkanApplication::FramePresent()
    {
        if (m_SwapChainRebuild)
            return;

        ImGui_ImplVulkanH_Window* wd = &m_MainWindowData;

        VkSemaphore render_complete_semaphore = wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;
        VkPresentInfoKHR info = {};
        info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &render_complete_semaphore;
        info.swapchainCount = 1;
        info.pSwapchains = &wd->Swapchain;
        info.pImageIndices = &wd->FrameIndex;

        const VkResult err = vkQueuePresentKHR(m_Queue, &info);
        if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        {
            m_SwapChainRebuild = true;
            return;
        }

        CheckVulkanResult(err);
        wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;
    }


    void VulkanApplication::SetupVulkanWindow(VkSurfaceKHR surface, int width, int height)
    {
        ImGui_ImplVulkanH_Window* wd = &m_MainWindowData;
        wd->Surface = surface;

        VkBool32 res;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, m_QueueFamily, wd->Surface, &res);
        if (res != VK_TRUE)
        {
            FatalError("vkGetPhysicalDeviceSurfaceSupportKHR return value was not VK_TRUE");
            return;
        }

        const VkFormat requestSurfaceImageFormat[] = {
            VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM
        };
        const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        wd->SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(m_PhysicalDevice,
                                                                  wd->Surface,
                                                                  requestSurfaceImageFormat,
                                                                  (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
                                                                  requestSurfaceColorSpace);

#ifdef APP_USE_UNLIMITED_FRAME_RATE
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR,
                                             VK_PRESENT_MODE_IMMEDIATE_KHR,
                                             VK_PRESENT_MODE_FIFO_KHR };
#else
        VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
#endif

        wd->PresentMode =
            ImGui_ImplVulkanH_SelectPresentMode(m_PhysicalDevice, wd->Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));

        IM_ASSERT(m_MinImageCount >= 2);
        ImGui_ImplVulkanH_CreateOrResizeWindow(
            m_Instance, m_PhysicalDevice, m_Device, wd, m_QueueFamily, m_Allocator, width, height, m_MinImageCount);
    }


    VkPhysicalDevice VulkanApplication::SelectAdapter()
    {
        uint32_t gpuCount;
        VkResult err = vkEnumeratePhysicalDevices(m_Instance, &gpuCount, nullptr);
        CheckVulkanResult(err);
        IM_ASSERT(gpuCount > 0);

        ImVector<VkPhysicalDevice> gpus;
        gpus.resize(gpuCount);
        err = vkEnumeratePhysicalDevices(m_Instance, &gpuCount, gpus.Data);
        CheckVulkanResult(err);

        for (VkPhysicalDevice& device : gpus)
        {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(device, &properties);
            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                return device;
        }

        if (gpuCount > 0)
            return gpus[0];

        return VK_NULL_HANDLE;
    }


    bool VulkanApplication::SetupBackend()
    {
        ImVector<const char*> instance_extensions;
        uint32_t extensions_count = 0;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
        for (uint32_t i = 0; i < extensions_count; i++)
            instance_extensions.push_back(glfw_extensions[i]);

        VkResult err;

        {
            VkInstanceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

            uint32_t properties_count;
            ImVector<VkExtensionProperties> properties;
            vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
            properties.resize(properties_count);
            err = vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, properties.Data);
            CheckVulkanResult(err);

            if (IsExtensionAvailable(properties, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
                instance_extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
            if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME))
            {
                instance_extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
                create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
            }
#endif

#ifdef QUINTE_USE_VULKAN_DEBUG_REPORT
            const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
            create_info.enabledLayerCount = 1;
            create_info.ppEnabledLayerNames = layers;
            instance_extensions.push_back("VK_EXT_debug_report");
#endif

            create_info.enabledExtensionCount = (uint32_t)instance_extensions.Size;
            create_info.ppEnabledExtensionNames = instance_extensions.Data;
            err = vkCreateInstance(&create_info, m_Allocator, &m_Instance);
            CheckVulkanResult(err);

#ifdef QUINTE_USE_VULKAN_DEBUG_REPORT
            auto vkCreateDebugReportCallbackEXT =
                (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugReportCallbackEXT");
            IM_ASSERT(vkCreateDebugReportCallbackEXT != nullptr);
            VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
            debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debug_report_ci.flags =
                VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
            debug_report_ci.pfnCallback = VulkanDebugReportCallback;
            debug_report_ci.pUserData = nullptr;
            err = vkCreateDebugReportCallbackEXT(m_Instance, &debug_report_ci, m_Allocator, &m_DebugReport);
            CheckVulkanResult(err);
#endif
        }

        m_PhysicalDevice = SelectAdapter();

        {
            uint32_t count;
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &count, nullptr);
            VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &count, queues);
            for (uint32_t i = 0; i < count; i++)
            {
                if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    m_QueueFamily = i;
                    break;
                }
            }

            free(queues);
            IM_ASSERT(m_QueueFamily != (uint32_t)-1);
        }

        {
            ImVector<const char*> device_extensions;
            device_extensions.push_back("VK_KHR_swapchain");

            uint32_t properties_count;
            ImVector<VkExtensionProperties> properties;
            vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &properties_count, nullptr);
            properties.resize(properties_count);
            vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &properties_count, properties.Data);
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
            if (IsExtensionAvailable(properties, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME))
                device_extensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

            const float queue_priority[] = { 1.0f };
            VkDeviceQueueCreateInfo queue_info[1] = {};
            queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_info[0].queueFamilyIndex = m_QueueFamily;
            queue_info[0].queueCount = 1;
            queue_info[0].pQueuePriorities = queue_priority;
            VkDeviceCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
            create_info.pQueueCreateInfos = queue_info;
            create_info.enabledExtensionCount = (uint32_t)device_extensions.Size;
            create_info.ppEnabledExtensionNames = device_extensions.Data;
            err = vkCreateDevice(m_PhysicalDevice, &create_info, m_Allocator, &m_Device);
            CheckVulkanResult(err);
            vkGetDeviceQueue(m_Device, m_QueueFamily, 0, &m_Queue);
        }

        {
            VkDescriptorPoolSize pool_sizes[] = {
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
            };
            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1;
            pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;
            err = vkCreateDescriptorPool(m_Device, &pool_info, m_Allocator, &m_DescriptorPool);
            CheckVulkanResult(err);
        }

        {
            VkSurfaceKHR surface;
            err = glfwCreateWindowSurface(m_Instance, m_pWindow, m_Allocator, &surface);
            CheckVulkanResult(err);

            int w, h;
            glfwGetFramebufferSize(m_pWindow, &w, &h);
            SetupVulkanWindow(surface, w, h);

            ImGui_ImplGlfw_InitForVulkan(m_pWindow, true);
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = m_Instance;
            init_info.PhysicalDevice = m_PhysicalDevice;
            init_info.Device = m_Device;
            init_info.QueueFamily = m_QueueFamily;
            init_info.Queue = m_Queue;
            init_info.PipelineCache = m_PipelineCache;
            init_info.DescriptorPool = m_DescriptorPool;
            init_info.RenderPass = m_MainWindowData.RenderPass;
            init_info.Subpass = 0;
            init_info.MinImageCount = m_MinImageCount;
            init_info.ImageCount = m_MainWindowData.ImageCount;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = m_Allocator;
            init_info.CheckVkResultFn = CheckVulkanResult;
            ImGui_ImplVulkan_Init(&init_info);
        }

        return true;
    }


    void VulkanApplication::BackendBeginFrame()
    {
        if (m_SwapChainRebuild)
        {
            int width, height;
            glfwGetFramebufferSize(m_pWindow, &width, &height);
            if (width > 0 && height > 0)
            {
                ImGui_ImplVulkan_SetMinImageCount(m_MinImageCount);
                ImGui_ImplVulkanH_CreateOrResizeWindow(m_Instance,
                                                       m_PhysicalDevice,
                                                       m_Device,
                                                       &m_MainWindowData,
                                                       m_QueueFamily,
                                                       m_Allocator,
                                                       width,
                                                       height,
                                                       m_MinImageCount);
                m_MainWindowData.FrameIndex = 0;
                m_SwapChainRebuild = false;
            }
        }

        ImGui_ImplVulkan_NewFrame();
    }


    void VulkanApplication::BackendEndFrame()
    {
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool isMinimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!isMinimized)
        {
            m_MainWindowData.ClearValue.color.float32[0] = m_ClearColor.x * m_ClearColor.w;
            m_MainWindowData.ClearValue.color.float32[1] = m_ClearColor.y * m_ClearColor.w;
            m_MainWindowData.ClearValue.color.float32[2] = m_ClearColor.z * m_ClearColor.w;
            m_MainWindowData.ClearValue.color.float32[3] = m_ClearColor.w;
            FrameRender(draw_data);
            FramePresent();
        }
    }


    VulkanApplication::~VulkanApplication()
    {
        const VkResult err = vkDeviceWaitIdle(m_Device);
        CheckVulkanResult(err);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        ImGui_ImplVulkanH_DestroyWindow(m_Instance, m_Device, &m_MainWindowData, m_Allocator);
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, m_Allocator);

#ifdef QUINTE_USE_VULKAN_DEBUG_REPORT
        auto vkDestroyDebugReportCallbackEXT =
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugReportCallbackEXT");
        vkDestroyDebugReportCallbackEXT(m_Instance, m_DebugReport, m_Allocator);
#endif

        vkDestroyDevice(m_Device, m_Allocator);
        vkDestroyInstance(m_Instance, m_Allocator);
    }
} // namespace quinte
