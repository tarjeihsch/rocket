#include "VulkanRHI.h"

#include "Common/RHI.h"
#include "VulkanTypes.h"
#include "Application.h"

#include <vector>
#include <Assertion.h>
#include <unordered_set>
#include <string_view>
#include <algorithm>
#include <unordered_map>

#if defined(_WIN32)
    #include <windows.h>
    #include <vulkan/vulkan_win32.h>
#endif

#if defined(__linux__)
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_xlib.h>
#endif

RHI::IRHI* CreateVulkanRHI()
{
    return new RHI::VulkanRHI();
}

namespace RHI
{
#if defined(_WIN32)
    static VkResult CreateSurfaceWin32(VkInstance instance, VkSurfaceKHR* surface, HWND hwnd, HINSTANCE window)
    {
        VkWin32SurfaceCreateInfoKHR surface_info = {};
        surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surface_info.hwnd = hwnd;
        surface_info.hinstance = GetModuleHandle(nullptr);

        auto fun = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));

        return fun(instance, &surface_info, nullptr, surface);
    }
#endif

#if defined(__linux__)
    static VkResult CreateSurfaceX11(VkInstance instance, VkSurfaceKHR* surface, void* display, void* window)
    {
        VkXlibSurfaceCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
        info.dpy = reinterpret_cast<Display*>(display);
        info.window = reinterpret_cast<Window>(window);

        auto fun = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR"));

        return fun(instance, &info, nullptr, surface);
    }
#endif
    
    static constexpr uint32_t           MAX_SHADER_BINDINGS                                 = 4U;
    
    static constexpr uint32_t           MAX_SHADER_RESOURCE_SAMPLED_TEXTURES                = 2048U;
    static constexpr uint32_t           MAX_SHADER_RESOURCE_STORAGE_TEXTURES                = 64U;
    static constexpr uint32_t           MAX_SHADER_RESOURCE_STORAGE_BUFFERS                 = 16U;
    static constexpr uint32_t           MAX_SHADER_RESOURCE_SAMPLERS                        = 1024U;

    static constexpr uint32_t           BINDING_SAMPLED_IMAGES                              = 0U;
    static constexpr uint32_t           BINDING_STORAGE_IMAGES                              = 1U;
    static constexpr uint32_t           BINDING_STORAGE_BUFFERS                             = 2U;
    static constexpr uint32_t           BINDING_SAMPLERS                                    = 3U;

    static constexpr uint32_t           MAX_BARRIER_COUNT                                   = 8;

    static VkInstance                   g_instance                                          = nullptr;
    static VkPhysicalDevice             g_physical_device                                   = nullptr;
    static VkDevice                     g_logical_device                                    = nullptr;
    
    static VmaAllocator                 g_memory_allocator                                  = nullptr;

    static uint32_t                     g_graphics_family                                   = UINT32_MAX;
    static uint32_t                     g_compute_family                                    = UINT32_MAX;
    static uint32_t                     g_transfer_family                                   = UINT32_MAX;

    static VkQueue                      g_graphics_queue                                    = nullptr;
    static VkQueue                      g_compute_queue                                     = nullptr;
    static VkQueue                      g_transfer_queue                                    = nullptr;

    static VkPipelineLayout  	        g_pipeline_layout                                   = nullptr;
    static VkDescriptorSetLayout        g_descriptor_set_layout                             = nullptr;
    static VkDescriptorSet              g_descriptor_set                                    = nullptr;
    static VkDescriptorPool             g_descriptor_pool                                   = nullptr;

    static GPUAdapter                   g_adapter                                           = {};

    static VkFence                      g_in_flight_fence                   [CONCURRENT_FRAME_COUNT]            = {};
    static VkQueryPool                  g_timeline_query_pool_data          [CONCURRENT_FRAME_COUNT]            = {};
    static VkQueryPool                  g_pipeline_query_pool_data          [CONCURRENT_FRAME_COUNT]            = {};
    static uint32_t                     g_query_pool_counter                [CONCURRENT_FRAME_COUNT]            = {};

    // The waits IRHI::Wait holds for the next submission to each queue.
    static std::vector<VkSemaphoreSubmitInfo> g_graphics_queue_waits;
    static std::vector<VkSemaphoreSubmitInfo> g_compute_queue_waits;
    static std::vector<VkSemaphoreSubmitInfo> g_transfer_queue_waits;

#if RK_DEBUG
    static VKAPI_ATTR VkBool32 VKAPI_CALL ExecDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
    {
        if (severity < VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            return false;
        }

        printf("[Vulkan] %s\n", CallbackData->pMessage);
        return VK_FALSE;
    }
#endif

    static void SetDebugNameInternal(VkObjectType type, uint64_t handle, const char* debug_name)
    {
        VkDebugUtilsObjectNameInfoEXT debug_info = {};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debug_info.objectType = type;
        debug_info.objectHandle = handle;
        debug_info.pObjectName = debug_name;
        RHI::vkSetDebugUtilsObjectNameEXT(g_logical_device, &debug_info);
    }

    static uint32_t GetQueueFamily(GPUQueueFlags queue)
    {
        switch (queue)
        {
            case GPUQueueFlag::Graphics:    return g_graphics_family;
            case GPUQueueFlag::Compute:     return g_compute_family;
            case GPUQueueFlag::Transfer:    return g_transfer_family;
            default:                        return VK_QUEUE_FAMILY_IGNORED;
        }
    }

    static VkQueue GetQueue(GPUQueueFlags queue)
    {
        switch (queue)
        {
            case GPUQueueFlag::Compute:     return g_compute_queue;
            case GPUQueueFlag::Transfer:    return g_transfer_queue;
            default:                        return g_graphics_queue;
        }
    }

    static std::vector<VkSemaphoreSubmitInfo>& GetQueueWaits(GPUQueueFlags queue)
    {
        switch (queue)
        {
            case GPUQueueFlag::Compute:     return g_compute_queue_waits;
            case GPUQueueFlag::Transfer:    return g_transfer_queue_waits;
            default:                        return g_graphics_queue_waits;
        }
    }

    // A barrier that hands a resource from one queue to another is recorded on both
    // queues. Between two families, the source queue releases the resource, so the
    // barrier has no second half there, and the destination queue acquires it,
    // where its first half only keeps the stage, which chains the barrier to the
    // fence wait that orders the acquire after the release. When the two queues
    // are one family, as when the transfer queue is the graphics queue, the source
    // queue's barrier is the whole handover and the destination queue records
    // nothing. Returns whether the queue records the barrier.
    static bool GetQueueTransferInfo(GPUQueueFlags queue, GPUQueueFlags src_queue, GPUQueueFlags dst_queue, VkAccessFlags2& src_access, VkPipelineStageFlags2& dst_stage, VkAccessFlags2& dst_access)
    {
        if (src_queue == dst_queue)
        {
            return true;
        }

        if (GetQueueFamily(src_queue) == GetQueueFamily(dst_queue))
        {
            return queue == src_queue;
        }

        if (queue == src_queue)
        {
            dst_stage = VK_PIPELINE_STAGE_2_NONE;
            dst_access = VK_ACCESS_2_NONE;
        }
        else
        {
            src_access = VK_ACCESS_2_NONE;
        }

        return true;
    }

    VulkanGPUBuffer::~VulkanGPUBuffer()
    {
        vmaDestroyBuffer(g_memory_allocator, buffer, allocation);
    }

    void VulkanGPUBuffer::SetDebugName(const char *debug_name)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(buffer), debug_name);
    }

    VulkanGPUTexture::~VulkanGPUTexture()
    {
        vmaDestroyImage(g_memory_allocator, texture, allocation);
        
        for (VkImageView view : views)
        {
            vkDestroyImageView(g_logical_device, view, nullptr);
        }

        for (VkImageView view : uavs)
        {
            vkDestroyImageView(g_logical_device, view, nullptr);
        }
    }

    void VulkanGPUTexture::SetDebugName(const char* debug_name)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_IMAGE, reinterpret_cast<uint64_t>(texture), debug_name);
    }

    VulkanGPUSampler::~VulkanGPUSampler()
    {
        RHI::vkDestroySampler(g_logical_device, sampler, nullptr);
    }

    void VulkanGPUSampler::SetDebugName(const char *debugName)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_SAMPLER, reinterpret_cast<uint64_t>(sampler), debugName);
    }

    VulkanGPUFence::~VulkanGPUFence()
    {
        RHI::vkDestroySemaphore(g_logical_device, semaphore, nullptr);
    }

    void VulkanGPUFence::SetDebugName(const char* debug_name)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_SEMAPHORE, reinterpret_cast<uint64_t>(semaphore), debug_name);
    }

    uint64_t VulkanGPUFence::GetCompletedValue() const
    {
        uint64_t value = 0;
        RHI::vkGetSemaphoreCounterValue(g_logical_device, semaphore, &value);
        return value;
    }

    void VulkanGPUFence::Wait(uint64_t value) const
    {
        VkSemaphoreWaitInfo wait_info = {};
        wait_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores = &semaphore;
        wait_info.pValues = &value;

        RHI::vkWaitSemaphores(g_logical_device, &wait_info, UINT64_MAX);
    }
    
    uint64_t VulkanGPUBuffer::GetVirtualAddress() const
    {
        VkBufferDeviceAddressInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.buffer = buffer;
        return RHI::vkGetBufferDeviceAddress(g_logical_device, &info);
    }

    void* VulkanGPUBuffer::Lock()
    {
        void* pointer = nullptr;
        VkResult result = vmaMapMemory(g_memory_allocator, allocation, &pointer);
        ensure(result == VK_SUCCESS);
        return pointer;
    }

    void VulkanGPUBuffer::Unlock()
    {
        vmaUnmapMemory(g_memory_allocator, allocation);
    }

    VulkanGPUShader::~VulkanGPUShader()
    {
        RHI::vkDestroyShaderModule(g_logical_device, module, nullptr);
    }

    VulkanGPUGraphicsPipeline::~VulkanGPUGraphicsPipeline()
    {
        RHI::vkDestroyPipeline(g_logical_device, pipeline, nullptr);
    }

    void VulkanGPUGraphicsPipeline::SetDebugName(const char* debug_name)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(pipeline), debug_name);
    }

    VulkanGPUComputePipeline::~VulkanGPUComputePipeline()
    {
        RHI::vkDestroyPipeline(g_logical_device, pipeline, nullptr);
    }

    void VulkanGPUComputePipeline::SetDebugName(const char* debug_name)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_PIPELINE, reinterpret_cast<uint64_t>(pipeline), debug_name);
    }

    VulkanGPUSwapchain::~VulkanGPUSwapchain()
    {
        RHI::vkDeviceWaitIdle(g_logical_device);

        for (uint32_t index = 0; index < image_count; index++)
        {
            textures[index]->texture = nullptr;
            textures[index]->allocation = nullptr;

            RHI::vkDestroySemaphore(g_logical_device, image_available_semaphores[index], nullptr);
            RHI::vkDestroySemaphore(g_logical_device, render_finished_semaphores[index], nullptr);

            delete textures[index];
        }

        RHI::vkDestroySwapchainKHR(g_logical_device, swapchain, nullptr);
        RHI::vkDestroySurfaceKHR(g_instance, surface, nullptr);
    }

    void VulkanGPUSwapchain::SetDebugName(const char* debug_name)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_SWAPCHAIN_KHR, reinterpret_cast<uint64_t>(swapchain), debug_name);
    }

    GPUTexture* VulkanGPUSwapchain::GetBackBufferTexture() const
    {
        return textures[current_image_index];
    }

    bool VulkanGPUSwapchain::AcquireNextBackBuffer()
    {
        RHI::vkWaitForFences(g_logical_device, 1, &g_in_flight_fence[CURRENT_FRAME], VK_TRUE, UINT64_MAX);

        VkResult result = RHI::vkAcquireNextImageKHR(g_logical_device, swapchain, UINT64_MAX, image_available_semaphores[CURRENT_FRAME], VK_NULL_HANDLE, &current_image_index);

        // No image was acquired, so no frame is submitted to signal the fence again.
        // Resetting it first would leave the next acquire waiting on it forever.
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            return false;
        }

        RHI::vkResetFences(g_logical_device, 1, &g_in_flight_fence[CURRENT_FRAME]);

        return true;
    }

    bool VulkanGPUSwapchain::PresentBackBuffer()
    {
        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_finished_semaphores[current_image_index];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &current_image_index;

        VkResult result = RHI::vkQueuePresentKHR(g_graphics_queue, &present_info);

        g_frame_counter++;

        return result == VK_SUCCESS;
    }

    void VulkanGPUSwapchain::ResizeBackBuffers()
    {
        RHI::vkDeviceWaitIdle(g_logical_device);

        for (uint32_t index = 0; index < image_count; ++index)
        {
            // The images belong to the swapchain, and the destructor destroys the view.
            textures[index]->texture = nullptr;
            textures[index]->allocation = nullptr;

            RHI::vkDestroySemaphore(g_logical_device, image_available_semaphores[index], nullptr);
            RHI::vkDestroySemaphore(g_logical_device, render_finished_semaphores[index], nullptr);

            delete textures[index];
        }

        free(image_available_semaphores);
        free(render_finished_semaphores);

        VkSurfaceCapabilitiesKHR surface_capabilities;
        RHI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physical_device, surface, &surface_capabilities);

        while (surface_capabilities.currentExtent.width == 0 || surface_capabilities.currentExtent.height == 0)
        {
            // Block the thread until the OS receives an event (like a restore or resize)
            Application::Get().WaitForEvents();
            
            // Re-check the capabilities after waking up
            RHI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physical_device, surface, &surface_capabilities);
        }

        extent = surface_capabilities.currentExtent;
        extent.width = std::clamp(extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);

        uint32_t available_texture_format_count = 0;
        RHI::vkGetPhysicalDeviceSurfaceFormatsKHR(g_physical_device, surface, &available_texture_format_count, nullptr);
        std::vector<VkSurfaceFormatKHR> available_texture_format_data(available_texture_format_count);
        RHI::vkGetPhysicalDeviceSurfaceFormatsKHR(g_physical_device, surface, &available_texture_format_count, available_texture_format_data.data());

        VkFormat texture_format = VK_FORMAT_B8G8R8A8_SRGB;
        VkColorSpaceKHR texture_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

        for (const auto& format : available_texture_format_data)
        {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                texture_format = format.format;
                texture_color_space = format.colorSpace;
                break;
            }
        }

        uint32_t new_image_count = image_count;
        if (new_image_count < surface_capabilities.minImageCount)
        {
            new_image_count = surface_capabilities.minImageCount;
        }
        if (surface_capabilities.maxImageCount > 0 && new_image_count > surface_capabilities.maxImageCount)
        {
            new_image_count = surface_capabilities.maxImageCount;
        }

        VkSwapchainKHR previous_swapchain = swapchain;

        VkSwapchainCreateInfoKHR swapchain_create_info = {};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = surface;
        swapchain_create_info.minImageCount = new_image_count;
        swapchain_create_info.imageFormat = texture_format;
        swapchain_create_info.imageColorSpace = texture_color_space;
        swapchain_create_info.imageExtent = extent;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.preTransform = surface_capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = previous_swapchain;

        VkResult result = RHI::vkCreateSwapchainKHR(g_logical_device, &swapchain_create_info, nullptr, &swapchain);
        ensure(result == VK_SUCCESS);

        RHI::vkDestroySwapchainKHR(g_logical_device, previous_swapchain, nullptr);

        RHI::vkGetSwapchainImagesKHR(g_logical_device, swapchain, &image_count, nullptr);
        std::vector<VkImage> images(image_count);
        RHI::vkGetSwapchainImagesKHR(g_logical_device, swapchain, &image_count, images.data());

        image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * image_count);
        render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * image_count);

        for (uint32_t index = 0; index < image_count; ++index)
        {
            textures[index] = new VulkanGPUTexture();
            textures[index]->texture = images[index];

            VkImageViewCreateInfo texture_view_create_info = {};
            texture_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            texture_view_create_info.image = images[index];
            texture_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            texture_view_create_info.format = texture_format;
            texture_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            texture_view_create_info.subresourceRange.baseMipLevel = 0;
            texture_view_create_info.subresourceRange.levelCount = 1;
            texture_view_create_info.subresourceRange.baseArrayLayer = 0;
            texture_view_create_info.subresourceRange.layerCount = 1;

            result = RHI::vkCreateImageView(g_logical_device, &texture_view_create_info, nullptr, &textures[index]->rtv);
            ensure(result == VK_SUCCESS);

            VkSemaphoreCreateInfo semaphore_create_info = {};
            semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            RHI::vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &image_available_semaphores[index]);
            RHI::vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &render_finished_semaphores[index]);
        }

        printf("Swapchain re-created: %d %d\nNew image count: %d\n", extent.width, extent.height, image_count);
    }

    Vector2i VulkanGPUSwapchain::GetBackBufferSize()
    {
        return Vector2i(extent.width, extent.height);
    }

    VulkanRHI::VulkanRHI()
    {
        RHI::LoadVulkanLibrary();

        VkApplicationInfo application_info = {};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = "Rocket";
        application_info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        application_info.pEngineName = "No Engine";
        application_info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        application_info.apiVersion = VK_API_VERSION_1_3;

#if RK_DEBUG
        VkDebugUtilsMessengerCreateInfoEXT debug_utils_messenger_create_info = {};
        debug_utils_messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_utils_messenger_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_utils_messenger_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_utils_messenger_create_info.pfnUserCallback = &ExecDebugCallback;
#endif

        std::vector<const char*> instance_extensions =
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#if defined(_WIN32)
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#elif defined(__linux__)
            VK_KHR_XLIB_SURFACE_EXTENSION_NAME
#endif
        };

#if RK_DEBUG
        std::vector<const char*> validation_extensions = 
        {
            "VK_LAYER_KHRONOS_validation",
        };
#endif

        std::vector<const char*> physical_device_extensions =
        {
            "VK_KHR_swapchain"
        };

#if RK_DEBUG
        std::vector<VkValidationFeatureEnableEXT> validation_feature_extensions =
        {
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
        };

        VkValidationFeaturesEXT validation_feature_data = {};
        validation_feature_data.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validation_feature_data.enabledValidationFeatureCount = validation_feature_extensions.size();
        validation_feature_data.pEnabledValidationFeatures = validation_feature_extensions.data();
        validation_feature_data.pNext = &debug_utils_messenger_create_info;
#endif

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &application_info;
        instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
        instance_create_info.enabledExtensionCount = instance_extensions.size();
#if RK_DEBUG
        instance_create_info.ppEnabledLayerNames = validation_extensions.data();
        instance_create_info.enabledLayerCount = validation_extensions.size();
        instance_create_info.pNext = &validation_feature_data;
#endif

        RHI::LoadVulkanGlobals();

        VkResult result = RHI::vkCreateInstance(&instance_create_info, nullptr, &g_instance);
        ensure(result == VK_SUCCESS, "Result: %d", result);

        RHI::LoadVulkanInstance(g_instance);

        uint32_t physical_device_count = 0;
        RHI::vkEnumeratePhysicalDevices(g_instance, &physical_device_count, nullptr);

        std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
        RHI::vkEnumeratePhysicalDevices(g_instance, &physical_device_count, physical_devices.data());

        std::vector<VkPhysicalDevice> discrete_physical_devices;
        std::vector<VkPhysicalDevice> integrated_physical_devices;
        std::vector<VkPhysicalDevice> virtual_physical_devices;

        for (VkPhysicalDevice physical_device : physical_devices)
        {
            VkPhysicalDeviceProperties GPUProperties;
            RHI::vkGetPhysicalDeviceProperties(physical_device, &GPUProperties);

            if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                discrete_physical_devices.push_back(physical_device);
            }

            if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
            {
                integrated_physical_devices.push_back(physical_device);
            }
        
            if (GPUProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
            {
                virtual_physical_devices.push_back(physical_device);
            }
        }

        std::vector<VkPhysicalDevice> sorted_physical_devices;
        sorted_physical_devices.insert(sorted_physical_devices.end(), discrete_physical_devices.begin(), discrete_physical_devices.end());
        sorted_physical_devices.insert(sorted_physical_devices.end(), integrated_physical_devices.begin(), integrated_physical_devices.end());
        sorted_physical_devices.insert(sorted_physical_devices.end(), virtual_physical_devices.begin(), virtual_physical_devices.end());

        for (VkPhysicalDevice physical_device : sorted_physical_devices)
        {
            uint32_t extension_count;
            RHI::vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

            std::vector<VkExtensionProperties> extensions(extension_count);
            RHI::vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());

            std::unordered_set<std::string_view> required_extensions;
            required_extensions.insert(physical_device_extensions.begin(), physical_device_extensions.end());

            for (const VkExtensionProperties& extension : extensions)
            {
                if (required_extensions.contains(extension.extensionName))
                {
                    required_extensions.erase(extension.extensionName);
                }
            }

            if (required_extensions.empty())
            {
                g_physical_device = physical_device;
                
                break;
            }
        }

        VkPhysicalDeviceProperties physical_device_properties;
        RHI::vkGetPhysicalDeviceProperties(g_physical_device, &physical_device_properties);

        VkPhysicalDeviceVulkan12Features query_device_features_1_2 = {};
        query_device_features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

        VkPhysicalDeviceVulkan13Features query_device_features_1_3 = {};
        query_device_features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        query_device_features_1_3.pNext = &query_device_features_1_2;

        VkPhysicalDeviceFeatures2 query_device_features = {};
        query_device_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        query_device_features.pNext = &query_device_features_1_3;

        RHI::vkGetPhysicalDeviceFeatures2(g_physical_device, &query_device_features);

        memcpy(g_adapter.device_name, physical_device_properties.deviceName, sizeof(g_adapter.device_name));
        g_adapter.device_id = physical_device_properties.deviceID;

        switch (physical_device_properties.vendorID)
        {
            case 0x10DE: g_adapter.vendor = GPUAdapterVendor::NVIDIA;   break;
            case 0x1002: g_adapter.vendor = GPUAdapterVendor::AMD;      break;
            case 0x8086: g_adapter.vendor = GPUAdapterVendor::INTEL;    break;
        }

        g_adapter.timestamp_period = physical_device_properties.limits.timestampPeriod;
        g_adapter.max_texture_dimension_2d = physical_device_properties.limits.maxImageDimension2D;
        g_adapter.max_texture_dimension_3d = physical_device_properties.limits.maxImageDimension3D;
        g_adapter.max_texture_array_layers = physical_device_properties.limits.maxImageArrayLayers;
        g_adapter.max_color_attachments = physical_device_properties.limits.maxColorAttachments;
        g_adapter.max_bind_groups = physical_device_properties.limits.maxBoundDescriptorSets;
        g_adapter.max_bind_group_entries = physical_device_properties.limits.maxDescriptorSetSampledImages;
        g_adapter.max_push_constant_size = physical_device_properties.limits.maxPushConstantsSize;
        g_adapter.max_compute_workgroup_size[0] = physical_device_properties.limits.maxComputeWorkGroupSize[0];
        g_adapter.max_compute_workgroup_size[1] = physical_device_properties.limits.maxComputeWorkGroupSize[1];
        g_adapter.max_compute_workgroup_size[2] = physical_device_properties.limits.maxComputeWorkGroupSize[2];
        g_adapter.max_compute_workgroup_count[0] = physical_device_properties.limits.maxComputeWorkGroupCount[0];
        g_adapter.max_compute_workgroup_count[1] = physical_device_properties.limits.maxComputeWorkGroupCount[1];
        g_adapter.max_compute_workgroup_count[2] = physical_device_properties.limits.maxComputeWorkGroupCount[2];
        g_adapter.max_sampled_textures = physical_device_properties.limits.maxDescriptorSetSampledImages;
        g_adapter.max_storage_textures = physical_device_properties.limits.maxDescriptorSetStorageImages;
        g_adapter.max_storage_buffers = physical_device_properties.limits.maxDescriptorSetStorageBuffers;
        g_adapter.max_uniform_buffers = physical_device_properties.limits.maxDescriptorSetUniformBuffers;
        g_adapter.max_samplers = physical_device_properties.limits.maxDescriptorSetSamplers;
        g_adapter.bindless_support = query_device_features_1_2.descriptorIndexing && query_device_features_1_2.runtimeDescriptorArray;
        g_adapter.indirect_draw_support = query_device_features.features.multiDrawIndirect;
        g_adapter.timestamp_query_support = physical_device_properties.limits.timestampComputeAndGraphics;

        VkPhysicalDeviceVulkan12Features request_device_features_1_2 = {};
        request_device_features_1_2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
        request_device_features_1_2.bufferDeviceAddress = VK_TRUE;
        request_device_features_1_2.bufferDeviceAddressCaptureReplay = VK_TRUE;
        request_device_features_1_2.descriptorIndexing = VK_TRUE;
        request_device_features_1_2.runtimeDescriptorArray = VK_TRUE;
        request_device_features_1_2.descriptorBindingPartiallyBound = VK_TRUE;
        request_device_features_1_2.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        request_device_features_1_2.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
        request_device_features_1_2.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
        request_device_features_1_2.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        request_device_features_1_2.timelineSemaphore = VK_TRUE;
        request_device_features_1_2.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
        request_device_features_1_2.hostQueryReset = VK_TRUE;
        request_device_features_1_2.drawIndirectCount = VK_TRUE;
        request_device_features_1_2.samplerFilterMinmax = VK_TRUE;

        VkPhysicalDeviceVulkan13Features request_device_features_1_3 = {};
        request_device_features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        request_device_features_1_3.pNext = &request_device_features_1_2;
        request_device_features_1_3.dynamicRendering = VK_TRUE;
        request_device_features_1_3.synchronization2 = VK_TRUE;

        VkPhysicalDeviceFeatures request_device_features = {};
        request_device_features.shaderInt64 = VK_TRUE;
        request_device_features.samplerAnisotropy = VK_TRUE;
        request_device_features.multiDrawIndirect = VK_TRUE;
        request_device_features.drawIndirectFirstInstance = VK_TRUE;
        request_device_features.shaderStorageImageArrayDynamicIndexing = VK_TRUE;
        request_device_features.pipelineStatisticsQuery = VK_TRUE;
        request_device_features.fillModeNonSolid = VK_TRUE;

        uint32_t queue_family_count = 0;
        RHI::vkGetPhysicalDeviceQueueFamilyProperties(g_physical_device, &queue_family_count, nullptr);

        VkQueueFamilyProperties queue_family_data[16];
        RHI::vkGetPhysicalDeviceQueueFamilyProperties(g_physical_device, &queue_family_count, queue_family_data);

        for (uint32_t index = 0; index < queue_family_count; index++)
        {
            VkQueueFlags queue_flags = queue_family_data[index].queueFlags;

            if ((queue_flags & VK_QUEUE_GRAPHICS_BIT) && g_graphics_family == UINT32_MAX)
            {
                g_graphics_family = index;
                g_adapter.timestamp_valid_bits = queue_family_data[index].timestampValidBits;
            }
            else if ((queue_flags & VK_QUEUE_COMPUTE_BIT) && !(queue_flags & VK_QUEUE_GRAPHICS_BIT) && g_compute_family == UINT32_MAX)
            {
                g_compute_family = index;
            }
            else if ((queue_flags & VK_QUEUE_TRANSFER_BIT) && !(queue_flags & VK_QUEUE_GRAPHICS_BIT) && !(queue_flags & VK_QUEUE_COMPUTE_BIT) && g_transfer_family == UINT32_MAX)
            {
                g_transfer_family = index;
            }
        }

        g_adapter.graphics_queue = g_graphics_family != UINT32_MAX;
        g_adapter.compute_queue = g_compute_family != UINT32_MAX;
        g_adapter.transfer_queue = g_transfer_family != UINT32_MAX;

        float queue_priority = 1.0f;
        VkDeviceQueueCreateInfo queue_create_infos[3] = {};
        uint32_t queue_create_count = 0;

        std::unordered_set<uint32_t> unique_queue_families;

        if (g_graphics_family != UINT32_MAX)
        {
            unique_queue_families.insert(g_graphics_family);
        }

        if (g_compute_family != UINT32_MAX)
        {
            unique_queue_families.insert(g_compute_family);
        }

        if (g_transfer_family != UINT32_MAX)
        {
            unique_queue_families.insert(g_transfer_family);
        }
        
        for (uint32_t queue_family : unique_queue_families)
        {
            queue_create_infos[queue_create_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_infos[queue_create_count].queueFamilyIndex = queue_family;
            queue_create_infos[queue_create_count].queueCount = 1;
            queue_create_infos[queue_create_count].pQueuePriorities = &queue_priority;
            queue_create_count++;
        }

        VkDeviceCreateInfo logical_device_create_info = {};
        logical_device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        logical_device_create_info.pQueueCreateInfos = queue_create_infos;
        logical_device_create_info.queueCreateInfoCount = queue_create_count;
        logical_device_create_info.pEnabledFeatures = &request_device_features;
        logical_device_create_info.pNext = &request_device_features_1_3;
        logical_device_create_info.ppEnabledExtensionNames = physical_device_extensions.data();
        logical_device_create_info.enabledExtensionCount = static_cast<uint32_t>(physical_device_extensions.size());
        logical_device_create_info.ppEnabledLayerNames = nullptr;
        logical_device_create_info.enabledLayerCount = 0;

        result = RHI::vkCreateDevice(g_physical_device, &logical_device_create_info, nullptr, &g_logical_device);
        ensure(result == VK_SUCCESS);

        RHI::LoadVulkanDevice(g_logical_device);

        RHI::vkGetDeviceQueue(g_logical_device, g_graphics_family, 0, &g_graphics_queue);

        // A GPU without a transfer family of its own copies on its graphics queue: a
        // transfer command list is created for the graphics family and submitted to
        // the graphics queue, which the transfer queue then names as well.
        if (g_transfer_family == UINT32_MAX)
        {
            g_transfer_family = g_graphics_family;
        }

        RHI::vkGetDeviceQueue(g_logical_device, g_transfer_family, 0, &g_transfer_queue);

        VmaVulkanFunctions allocator_functions = {};
        allocator_functions.vkGetInstanceProcAddr = RHI::vkGetInstanceProcAddr;
        allocator_functions.vkGetDeviceProcAddr = RHI::vkGetDeviceProcAddr;
        allocator_functions.vkGetPhysicalDeviceProperties = RHI::vkGetPhysicalDeviceProperties;
        allocator_functions.vkGetPhysicalDeviceMemoryProperties = RHI::vkGetPhysicalDeviceMemoryProperties;
        allocator_functions.vkAllocateMemory = RHI::vkAllocateMemory;
        allocator_functions.vkFreeMemory = RHI::vkFreeMemory;
        allocator_functions.vkMapMemory = RHI::vkMapMemory;
        allocator_functions.vkUnmapMemory = RHI::vkUnmapMemory;
        allocator_functions.vkFlushMappedMemoryRanges = RHI::vkFlushMappedMemoryRanges;
        allocator_functions.vkInvalidateMappedMemoryRanges = RHI::vkInvalidateMappedMemoryRanges;
        allocator_functions.vkBindBufferMemory = RHI::vkBindBufferMemory;
        allocator_functions.vkBindImageMemory = RHI::vkBindImageMemory;
        allocator_functions.vkGetBufferMemoryRequirements = RHI::vkGetBufferMemoryRequirements;
        allocator_functions.vkGetImageMemoryRequirements = RHI::vkGetImageMemoryRequirements;
        allocator_functions.vkCreateBuffer = RHI::vkCreateBuffer;
        allocator_functions.vkDestroyBuffer = RHI::vkDestroyBuffer;
        allocator_functions.vkCreateImage = RHI::vkCreateImage;
        allocator_functions.vkDestroyImage = RHI::vkDestroyImage;
        allocator_functions.vkCmdCopyBuffer = RHI::vkCmdCopyBuffer;
        allocator_functions.vkGetBufferMemoryRequirements2KHR = RHI::vkGetBufferMemoryRequirements2;
        allocator_functions.vkGetImageMemoryRequirements2KHR = RHI::vkGetImageMemoryRequirements2;
        allocator_functions.vkBindBufferMemory2KHR = RHI::vkBindBufferMemory2;
        allocator_functions.vkBindImageMemory2KHR = RHI::vkBindImageMemory2;
        allocator_functions.vkGetPhysicalDeviceMemoryProperties2KHR = RHI::vkGetPhysicalDeviceMemoryProperties2;
        allocator_functions.vkGetDeviceBufferMemoryRequirements = RHI::vkGetDeviceBufferMemoryRequirements;
        allocator_functions.vkGetDeviceImageMemoryRequirements = RHI::vkGetDeviceImageMemoryRequirements;

        VmaAllocatorCreateInfo memory_allocator_create_info = {};
        memory_allocator_create_info.physicalDevice = g_physical_device;
        memory_allocator_create_info.device = g_logical_device;
        memory_allocator_create_info.instance = g_instance;
        memory_allocator_create_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        memory_allocator_create_info.pVulkanFunctions = &allocator_functions;

        result = vmaCreateAllocator(&memory_allocator_create_info, &g_memory_allocator);
        ensure(result == VK_SUCCESS);

        VkDescriptorBindingFlags descriptor_binding_flags[MAX_SHADER_BINDINGS];
        std::fill(descriptor_binding_flags, descriptor_binding_flags + MAX_SHADER_BINDINGS, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);

        VkDescriptorSetLayoutBindingFlagsCreateInfo descriptor_set_layout_binding_flags_create_info = {};
        descriptor_set_layout_binding_flags_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        descriptor_set_layout_binding_flags_create_info.bindingCount = MAX_SHADER_BINDINGS;
        descriptor_set_layout_binding_flags_create_info.pBindingFlags = descriptor_binding_flags;

        VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[MAX_SHADER_BINDINGS] = {};
        descriptor_set_layout_bindings[BINDING_SAMPLED_IMAGES].binding = BINDING_SAMPLED_IMAGES;
        descriptor_set_layout_bindings[BINDING_SAMPLED_IMAGES].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor_set_layout_bindings[BINDING_SAMPLED_IMAGES].descriptorCount = std::min<uint32_t>(g_adapter.max_sampled_textures, MAX_SHADER_RESOURCE_SAMPLED_TEXTURES);
        descriptor_set_layout_bindings[BINDING_SAMPLED_IMAGES].stageFlags = VK_SHADER_STAGE_ALL;
        
        descriptor_set_layout_bindings[BINDING_STORAGE_IMAGES].binding = BINDING_STORAGE_IMAGES;
        descriptor_set_layout_bindings[BINDING_STORAGE_IMAGES].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptor_set_layout_bindings[BINDING_STORAGE_IMAGES].descriptorCount = std::min<uint32_t>(g_adapter.max_storage_textures, MAX_SHADER_RESOURCE_STORAGE_TEXTURES);
        descriptor_set_layout_bindings[BINDING_STORAGE_IMAGES].stageFlags = VK_SHADER_STAGE_ALL;
        
        descriptor_set_layout_bindings[BINDING_STORAGE_BUFFERS].binding = BINDING_STORAGE_BUFFERS;
        descriptor_set_layout_bindings[BINDING_STORAGE_BUFFERS].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor_set_layout_bindings[BINDING_STORAGE_BUFFERS].descriptorCount = std::min<uint32_t>(g_adapter.max_storage_buffers, MAX_SHADER_RESOURCE_STORAGE_BUFFERS);
        descriptor_set_layout_bindings[BINDING_STORAGE_BUFFERS].stageFlags = VK_SHADER_STAGE_ALL;
        
        descriptor_set_layout_bindings[BINDING_SAMPLERS].binding = BINDING_SAMPLERS;
        descriptor_set_layout_bindings[BINDING_SAMPLERS].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptor_set_layout_bindings[BINDING_SAMPLERS].descriptorCount = std::min<uint32_t>(g_adapter.max_samplers, MAX_SHADER_RESOURCE_SAMPLERS);
        descriptor_set_layout_bindings[BINDING_SAMPLERS].stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.pNext = &descriptor_set_layout_binding_flags_create_info;
        descriptor_set_layout_create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        descriptor_set_layout_create_info.bindingCount = MAX_SHADER_BINDINGS;
        descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings;

        result = RHI::vkCreateDescriptorSetLayout(g_logical_device, &descriptor_set_layout_create_info, nullptr, &g_descriptor_set_layout);
        ensure(result == VK_SUCCESS);

        VkDescriptorPoolSize descriptor_pool_size_data[MAX_SHADER_BINDINGS] = {};
        descriptor_pool_size_data[BINDING_SAMPLED_IMAGES] = { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, std::min<uint32_t>(g_adapter.max_sampled_textures, MAX_SHADER_RESOURCE_SAMPLED_TEXTURES) };
        descriptor_pool_size_data[BINDING_STORAGE_IMAGES] = { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, std::min<uint32_t>(g_adapter.max_storage_textures, MAX_SHADER_RESOURCE_STORAGE_TEXTURES) };
        descriptor_pool_size_data[BINDING_STORAGE_BUFFERS] = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, std::min<uint32_t>(g_adapter.max_storage_buffers, MAX_SHADER_RESOURCE_STORAGE_BUFFERS) };
        descriptor_pool_size_data[BINDING_SAMPLERS] = { VK_DESCRIPTOR_TYPE_SAMPLER, std::min<uint32_t>(g_adapter.max_samplers, MAX_SHADER_RESOURCE_SAMPLERS) };

        VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        descriptor_pool_create_info.maxSets = 1;
        descriptor_pool_create_info.poolSizeCount = std::size(descriptor_pool_size_data);
        descriptor_pool_create_info.pPoolSizes = descriptor_pool_size_data;

        result = RHI::vkCreateDescriptorPool(g_logical_device, &descriptor_pool_create_info, nullptr, &g_descriptor_pool);
        ensure(result == VK_SUCCESS);

        VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorPool = g_descriptor_pool;
        descriptor_set_allocate_info.descriptorSetCount = 1;
        descriptor_set_allocate_info.pSetLayouts = &g_descriptor_set_layout;

        result = RHI::vkAllocateDescriptorSets(g_logical_device, &descriptor_set_allocate_info, &g_descriptor_set);
        ensure(result == VK_SUCCESS);

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = VK_SHADER_STAGE_ALL;
        push_constant_range.size = g_adapter.max_push_constant_size;

        VkPipelineLayoutCreateInfo pipeline_layout_layout_info = {};
        pipeline_layout_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_layout_info.pSetLayouts = &g_descriptor_set_layout;
        pipeline_layout_layout_info.setLayoutCount = 1;
        pipeline_layout_layout_info.pushConstantRangeCount = 1;
        pipeline_layout_layout_info.pPushConstantRanges = &push_constant_range;

        result = RHI::vkCreatePipelineLayout(g_logical_device, &pipeline_layout_layout_info, nullptr, &g_pipeline_layout);
        ensure(result == VK_SUCCESS);

        for (uint32_t index = 0; index < CONCURRENT_FRAME_COUNT; ++index)
        {
            VkFenceCreateInfo fence_create_info = {};
            fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            RHI::vkCreateFence(g_logical_device, &fence_create_info, nullptr, &g_in_flight_fence[index]);

            VkQueryPoolCreateInfo timeline_query_pool_create_info = {};
            timeline_query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            timeline_query_pool_create_info.queryType = VK_QUERY_TYPE_TIMESTAMP;
            timeline_query_pool_create_info.queryCount = MAX_TIMESTAMP_COUNT;
            RHI::vkCreateQueryPool(g_logical_device, &timeline_query_pool_create_info, nullptr, &g_timeline_query_pool_data[index]);
            RHI::vkResetQueryPool(g_logical_device, g_timeline_query_pool_data[index], 0, MAX_TIMESTAMP_COUNT);

            VkQueryPoolCreateInfo pipeline_query_pool_create_info = {};
            pipeline_query_pool_create_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
            pipeline_query_pool_create_info.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
            pipeline_query_pool_create_info.queryCount = 1;
            pipeline_query_pool_create_info.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
            RHI::vkCreateQueryPool(g_logical_device, &pipeline_query_pool_create_info, nullptr, &g_pipeline_query_pool_data[index]);
            RHI::vkResetQueryPool(g_logical_device, g_pipeline_query_pool_data[index], 0, 1);
        }

        std::printf("[RHI] GPU:\n\tVendor: %s\n\tName: %s\n\tID: %d\n", LexToString(g_adapter.vendor), g_adapter.device_name, g_adapter.device_id);
    }

    VulkanRHI::~VulkanRHI()
    {
        for (uint32_t index = 0; index < CONCURRENT_FRAME_COUNT; ++index)
        {
            RHI::vkDestroyFence(g_logical_device, g_in_flight_fence[index], nullptr);
            RHI::vkDestroyQueryPool(g_logical_device, g_timeline_query_pool_data[index], nullptr);
            RHI::vkDestroyQueryPool(g_logical_device, g_pipeline_query_pool_data[index], nullptr);
        }

        RHI::vkDestroyDescriptorPool(g_logical_device, g_descriptor_pool, nullptr);
        RHI::vkDestroyDescriptorSetLayout(g_logical_device, g_descriptor_set_layout, nullptr);
        RHI::vkDestroyPipelineLayout(g_logical_device, g_pipeline_layout, nullptr);

        vmaDestroyAllocator(g_memory_allocator);

        RHI::vkDestroyDevice(g_logical_device, nullptr);
        RHI::vkDestroyInstance(g_instance, nullptr);
    }

    GPUAdapter VulkanRHI::GetAdapater() const
    {
        return GPUAdapter();
    }
    
    void VulkanRHI::Submit(GPUCommandList* command_list, GPUSwapchain* swapchain)
    {
        VkCommandBufferSubmitInfo command_buffer_submit_info = {};
        command_buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_buffer_submit_info.commandBuffer = static_cast<VulkanGPUCommandList*>(command_list)->command_buffer_data[CURRENT_FRAME];

        VkSemaphoreSubmitInfo wait_semaphore_info = {};
        wait_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        wait_semaphore_info.semaphore = ((VulkanGPUSwapchain*)swapchain)->image_available_semaphores[CURRENT_FRAME];
        wait_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSemaphoreSubmitInfo signal_semaphore_info = {};
        signal_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signal_semaphore_info.semaphore = ((VulkanGPUSwapchain*)swapchain)->render_finished_semaphores[((VulkanGPUSwapchain*)swapchain)->current_image_index];
        signal_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;

        // The frame waits for its back buffer and for every fence IRHI::Wait added.
        g_graphics_queue_waits.push_back(wait_semaphore_info);

        VkSubmitInfo2 submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &command_buffer_submit_info;
        submit_info.waitSemaphoreInfoCount = static_cast<uint32_t>(g_graphics_queue_waits.size());
        submit_info.pWaitSemaphoreInfos = g_graphics_queue_waits.data();
        submit_info.signalSemaphoreInfoCount = 1;
        submit_info.pSignalSemaphoreInfos = &signal_semaphore_info;

        VkResult result = RHI::vkQueueSubmit2(g_graphics_queue, 1, &submit_info, g_in_flight_fence[CURRENT_FRAME]);
        ensure(result == VK_SUCCESS);

        g_graphics_queue_waits.clear();
    }

    void VulkanRHI::SubmitImmediate(GPUCommandList *command_list)
    {
        VkCommandBufferSubmitInfo command_buffer_submit_info = {};
        command_buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_buffer_submit_info.commandBuffer = static_cast<VulkanGPUCommandList*>(command_list)->command_buffer_data[CURRENT_FRAME];

        VkSubmitInfo2 submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &command_buffer_submit_info;

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = 0;
        
        VkFence fence;
        RHI::vkCreateFence(g_logical_device, &fence_info, nullptr, &fence);

        VkResult result = RHI::vkQueueSubmit2(g_graphics_queue, 1, &submit_info, fence);
        ensure(result == VK_SUCCESS);

        RHI::vkWaitForFences(g_logical_device, 1, &fence, VK_TRUE, UINT64_MAX);
        RHI::vkDestroyFence(g_logical_device, fence, nullptr);
    }

    void VulkanRHI::Submit(GPUCommandList* command_list, GPUFence* fence, uint64_t value)
    {
        const VulkanGPUCommandList* vulkan_command_list = static_cast<const VulkanGPUCommandList*>(command_list);
        std::vector<VkSemaphoreSubmitInfo>& wait_semaphore_infos = GetQueueWaits(vulkan_command_list->queue_flags);

        VkCommandBufferSubmitInfo command_buffer_submit_info = {};
        command_buffer_submit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_buffer_submit_info.commandBuffer = vulkan_command_list->command_buffer_data[CURRENT_FRAME];

        VkSemaphoreSubmitInfo signal_semaphore_info = {};
        signal_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        signal_semaphore_info.semaphore = static_cast<const VulkanGPUFence*>(fence)->semaphore;
        signal_semaphore_info.value = value;
        signal_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkSubmitInfo2 submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.commandBufferInfoCount = 1;
        submit_info.pCommandBufferInfos = &command_buffer_submit_info;
        submit_info.waitSemaphoreInfoCount = static_cast<uint32_t>(wait_semaphore_infos.size());
        submit_info.pWaitSemaphoreInfos = wait_semaphore_infos.data();
        submit_info.signalSemaphoreInfoCount = 1;
        submit_info.pSignalSemaphoreInfos = &signal_semaphore_info;

        VkResult result = RHI::vkQueueSubmit2(GetQueue(vulkan_command_list->queue_flags), 1, &submit_info, VK_NULL_HANDLE);
        ensure(result == VK_SUCCESS);

        wait_semaphore_infos.clear();
    }

    void VulkanRHI::Wait(GPUQueueFlags queue, GPUFence* fence, uint64_t value)
    {
        VkSemaphoreSubmitInfo wait_semaphore_info = {};
        wait_semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        wait_semaphore_info.semaphore = static_cast<const VulkanGPUFence*>(fence)->semaphore;
        wait_semaphore_info.value = value;
        wait_semaphore_info.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        GetQueueWaits(queue).push_back(wait_semaphore_info);
    }

    void VulkanRHI::WaitIdle()
    {
        RHI::vkDeviceWaitIdle(g_logical_device);
    }

    GPUTimingQueryResult VulkanRHI::GetTimingQueryResult()
    {
        GPUTimingQueryResult timing_query_result = {};

        double timestamps[MAX_TIMESTAMP_COUNT];
        const uint32_t count = GetTimestampResults(timestamps, MAX_TIMESTAMP_COUNT);

        if (count >= 2)
        {
            timing_query_result.gpu_time_ms = timestamps[count - 1];
        }

        return timing_query_result;
    }

    uint32_t VulkanRHI::GetTimestampResults(double* milliseconds, uint32_t capacity)
    {
        if (g_frame_counter < CONCURRENT_FRAME_COUNT)
        {
            return 0;
        }

        const uint32_t count = g_query_pool_counter[CURRENT_FRAME];
        ensure(count <= capacity);

        uint64_t timestamps[MAX_TIMESTAMP_COUNT];
        RHI::vkGetQueryPoolResults(g_logical_device, g_timeline_query_pool_data[CURRENT_FRAME], 0, count, sizeof(timestamps), timestamps, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

        // Only the low bits of a timestamp count, so the difference wraps within them.
        const uint64_t mask = g_adapter.timestamp_valid_bits >= 64 ? ~0ULL : (1ULL << g_adapter.timestamp_valid_bits) - 1;

        for (uint32_t index = 0; index < count; index++)
        {
            milliseconds[index] = ((timestamps[index] - timestamps[0]) & mask) * g_adapter.timestamp_period / 1000000.0;
        }

        return count;
    }

    GPUPipelineQueryResult VulkanRHI::GetPipelineQueryResult()
    {
        GPUPipelineQueryResult pipeline_query_result = {};
        if (g_frame_counter < CONCURRENT_FRAME_COUNT)
        {
            return pipeline_query_result;
        }

        RHI::vkGetQueryPoolResults(g_logical_device, g_pipeline_query_pool_data[CURRENT_FRAME], 0, 1, sizeof(pipeline_query_result), &pipeline_query_result, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
        return pipeline_query_result;
    }
    
    GPUBuffer *VulkanRHI::CreateBuffer(const GPUBufferDesc &buffer_desc)
    {
        VulkanGPUBuffer* buffer = new VulkanGPUBuffer();
        
        VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Vertex) buffer_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Index) buffer_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Storage) buffer_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Uniform) buffer_usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Indirect) buffer_usage_flags |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

        VmaAllocationCreateFlags memory_allocation_flags = 0;
        switch (buffer_desc.heap_type)
        {
            case GPUHeapType::Upload: memory_allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; break;
            case GPUHeapType::Readback: memory_allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT; break;
            default: break;
        }

        VkBufferCreateInfo buffer_create_info = {};
        buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_create_info.size = buffer_desc.size;
        buffer_create_info.usage = buffer_usage_flags;

        // The transfer queue writes ranges of a buffer while the graphics queue reads
        // the rest, as with a pool of geometry, which handing the whole buffer between
        // them cannot express. So when the transfer queue has a family of its own,
        // both families share every buffer.
        const uint32_t queue_families[] = { g_graphics_family, g_transfer_family };

        if (g_transfer_family != g_graphics_family)
        {
            buffer_create_info.sharingMode = VK_SHARING_MODE_CONCURRENT;
            buffer_create_info.queueFamilyIndexCount = 2;
            buffer_create_info.pQueueFamilyIndices = queue_families;
        }

        VmaAllocationCreateInfo memory_allocation_info = {};
        memory_allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
        memory_allocation_info.flags = memory_allocation_flags;

        VkResult result = vmaCreateBuffer(g_memory_allocator, &buffer_create_info, &memory_allocation_info, &buffer->buffer, &buffer->allocation, nullptr);
        ensure(result == VK_SUCCESS);

        return buffer;
    }
    
    GPUTexture* VulkanRHI::CreateTexture(const GPUTextureDesc& texture_desc)
    {
        VulkanGPUTexture* texture = new VulkanGPUTexture();

        VkFormat format = GetVkFormat(texture_desc.format);
        VkImageUsageFlags image_usage_flags = 0;
        VkImageType texture_type = VK_IMAGE_TYPE_2D;
        VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D;
        VkImageCreateFlags texture_flags = 0;

        if (texture_desc.flags & GPUTextureFlag::RenderTarget) image_usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (texture_desc.flags & GPUTextureFlag::DepthTarget) image_usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (texture_desc.flags & GPUTextureFlag::SRV) image_usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (texture_desc.flags & GPUTextureFlag::UAV) image_usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (texture_desc.flags & GPUTextureFlag::TransferSrc) image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (texture_desc.flags & GPUTextureFlag::TransferDst) image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        switch (texture_desc.dimension)
        {
            case GPUTextureDimension::Texture1D:   
            {
                texture_type = VK_IMAGE_TYPE_1D;
                view_type = VK_IMAGE_VIEW_TYPE_1D;
                break;
            }
            case GPUTextureDimension::Texture2D:   
            {
                texture_type = VK_IMAGE_TYPE_2D;
                view_type = VK_IMAGE_VIEW_TYPE_2D;
                break;
            }
            case GPUTextureDimension::Texture3D:   
            {
                texture_type = VK_IMAGE_TYPE_3D;
                view_type = VK_IMAGE_VIEW_TYPE_3D;
                break;
            }
            case GPUTextureDimension::TextureCube:
            {
                texture_type = VK_IMAGE_TYPE_2D;
                view_type = VK_IMAGE_VIEW_TYPE_CUBE;
                texture_flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
                break;
            }
            default: break;
        }

        VkImageCreateInfo texture_create_info = {};
        texture_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        texture_create_info.format = format;
        texture_create_info.extent = { texture_desc.width, texture_desc.height, texture_desc.depth };
        texture_create_info.mipLevels = texture_desc.mip_level_count;
        texture_create_info.arrayLayers = texture_desc.layer_count;
        texture_create_info.samples = static_cast<VkSampleCountFlagBits>(texture_desc.sample_count);
        texture_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        texture_create_info.usage = image_usage_flags;
        texture_create_info.imageType = texture_type;
        texture_create_info.flags = texture_flags;

        // The image is created as UNORM and viewed as either, which the format list
        // declares.
        const VkFormat srgb_format = (texture_desc.flags & GPUTextureFlag::SRGBView) != 0 ? GetVkSRGBFormat(texture_desc.format) : VK_FORMAT_UNDEFINED;
        const VkFormat view_formats[] = { format, srgb_format };

        VkImageFormatListCreateInfo format_list_create_info = {};
        format_list_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO;
        format_list_create_info.viewFormatCount = 2;
        format_list_create_info.pViewFormats = view_formats;

        if (srgb_format != VK_FORMAT_UNDEFINED)
        {
            texture_create_info.flags |= VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT;
            texture_create_info.pNext = &format_list_create_info;
        }

        VmaAllocationCreateInfo memory_allocation_info = {};
        memory_allocation_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VkResult result = vmaCreateImage(g_memory_allocator, &texture_create_info, &memory_allocation_info, &texture->texture, &texture->allocation, nullptr);
        ensure(result == VK_SUCCESS);

        VkImageAspectFlags aspect_flag = (texture_desc.flags & GPUTextureFlag::DepthTarget) != 0 ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageViewCreateInfo texture_view_create_info = {};
        texture_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        texture_view_create_info.image = texture->texture;
        texture_view_create_info.viewType = view_type;
        texture_view_create_info.format = format;
        texture_view_create_info.subresourceRange.baseArrayLayer = 0;
        texture_view_create_info.subresourceRange.layerCount = texture_desc.layer_count;
        texture_view_create_info.subresourceRange.baseMipLevel = 0;

        if (texture_desc.flags & GPUTextureFlag::RenderTarget)
        {
            texture_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            texture_view_create_info.subresourceRange.levelCount = 1;
            RHI::vkCreateImageView(g_logical_device, &texture_view_create_info, nullptr, &texture->rtv);
        }

        if (texture_desc.flags & GPUTextureFlag::DepthTarget)
        {
            texture_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            texture_view_create_info.subresourceRange.levelCount = 1;
            RHI::vkCreateImageView(g_logical_device, &texture_view_create_info, nullptr, &texture->dsv);
        }

        if (texture_desc.flags & GPUTextureFlag::SRV)
        {
            texture_view_create_info.subresourceRange.aspectMask = aspect_flag;
            texture_view_create_info.subresourceRange.levelCount = texture_desc.mip_level_count;
            RHI::vkCreateImageView(g_logical_device, &texture_view_create_info, nullptr, &texture->srv);

            if (srgb_format != VK_FORMAT_UNDEFINED)
            {
                // The sRGB format cannot be stored to, so the view is for sampling only.
                VkImageViewUsageCreateInfo view_usage_create_info = {};
                view_usage_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_USAGE_CREATE_INFO;
                view_usage_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

                texture_view_create_info.pNext = &view_usage_create_info;
                texture_view_create_info.format = srgb_format;
                RHI::vkCreateImageView(g_logical_device, &texture_view_create_info, nullptr, &texture->srgb_srv);

                texture_view_create_info.pNext = nullptr;
                texture_view_create_info.format = format;
            }
        }

        if (texture_desc.flags & GPUTextureFlag::UAV)
        {
            texture_view_create_info.subresourceRange.aspectMask = aspect_flag;
            texture_view_create_info.subresourceRange.levelCount = 1;

            texture->uavs.resize(texture_desc.mip_level_count);
            for (uint32_t mip = 0; mip < texture_desc.mip_level_count; ++mip)
            {
                texture_view_create_info.subresourceRange.baseMipLevel = mip;
                RHI::vkCreateImageView(g_logical_device, &texture_view_create_info, nullptr, &texture->uavs[mip]);
            }
        }

        return texture;
    }

    GPUSampler* VulkanRHI::CreateSampler(const GPUSamplerDesc& samplerDesc)
    {
        VkSamplerCreateInfo samplerCreateInfo = {};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.minFilter = samplerDesc.minFilter == GPUFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        samplerCreateInfo.magFilter = samplerDesc.magFilter == GPUFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        samplerCreateInfo.mipmapMode = samplerDesc.mipFilter == GPUFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerCreateInfo.addressModeU = GetAddressMode(samplerDesc.addressU);
        samplerCreateInfo.addressModeV = GetAddressMode(samplerDesc.addressV);
        samplerCreateInfo.addressModeW = GetAddressMode(samplerDesc.addressW);
        samplerCreateInfo.mipLodBias = samplerDesc.mipLodBias;
        samplerCreateInfo.minLod = samplerDesc.minLod;
        samplerCreateInfo.maxLod = samplerDesc.maxLod;
        samplerCreateInfo.anisotropyEnable = samplerDesc.anisotropyEnable ? VK_TRUE : VK_FALSE;
        samplerCreateInfo.maxAnisotropy = samplerDesc.maxAnisotropy;
        samplerCreateInfo.compareEnable = samplerDesc.compareEnable ? VK_TRUE : VK_FALSE;
        samplerCreateInfo.compareOp = GetCompareOp(samplerDesc.compareOp);
        samplerCreateInfo.borderColor = GetBorderColor(samplerDesc.borderColor);

        VkSamplerReductionModeCreateInfo reductionModeCreateInfo = {};
        reductionModeCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO;
        reductionModeCreateInfo.reductionMode = GetReductionMode(samplerDesc.reductionMode);
        samplerCreateInfo.pNext = &reductionModeCreateInfo;

        VulkanGPUSampler* vkSampler = new VulkanGPUSampler();

        VkResult result = RHI::vkCreateSampler(g_logical_device, &samplerCreateInfo, nullptr, &vkSampler->sampler);
        ensure(result == VK_SUCCESS);

        return vkSampler;
    }
    
    GPUGraphicsPipeline* VulkanRHI::CreateGraphicsPipeline(const GPUGraphicsPipelineDesc &pipeline_desc)
    {
        VulkanGPUGraphicsPipeline* graphicsPipeline = new VulkanGPUGraphicsPipeline();
        
        const VulkanGPUShader* native_vertex_shader = static_cast<const VulkanGPUShader*>(pipeline_desc.vertex_shader);
        const VulkanGPUShader* native_fragment_shader = static_cast<const VulkanGPUShader*>(pipeline_desc.fragment_shader);

        VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info[2] = {};
        pipeline_shader_stage_create_info[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_info[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pipeline_shader_stage_create_info[0].module = native_vertex_shader->module;
        pipeline_shader_stage_create_info[0].pName = "VSMain";
        
        pipeline_shader_stage_create_info[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipeline_shader_stage_create_info[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pipeline_shader_stage_create_info[1].module = native_fragment_shader->module;
        pipeline_shader_stage_create_info[1].pName = "PSMain";

        // No vertex input - route through vertex pulling
        VkPipelineVertexInputStateCreateInfo pipeline_vertex_input_create_info = {};
        pipeline_vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo pipeline_input_assembly_create_info = {};
        pipeline_input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipeline_input_assembly_create_info.topology = GetNativeTopology(pipeline_desc.topology);

        VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info = {};
        pipeline_dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipeline_dynamic_state_create_info.dynamicStateCount = std::size(dynamic_states);
        pipeline_dynamic_state_create_info.pDynamicStates = dynamic_states;

        VkPipelineViewportStateCreateInfo pipeline_viewport_state_create_info = {};
        pipeline_viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipeline_viewport_state_create_info.viewportCount = 1;
        pipeline_viewport_state_create_info.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {};
        pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipeline_rasterization_state_create_info.cullMode = GetNativeCullMode(pipeline_desc.cull_mode);
        pipeline_rasterization_state_create_info.frontFace = GetNativeFrontFace(pipeline_desc.front_face);
        pipeline_rasterization_state_create_info.polygonMode = GetNativePolygonMode(pipeline_desc.polygon_mode);
        pipeline_rasterization_state_create_info.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info = {};
        pipeline_multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipeline_multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo pipeline_depth_stencil_state_create_info = {};
        pipeline_depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipeline_depth_stencil_state_create_info.depthTestEnable = pipeline_desc.depth_test_enable;
        pipeline_depth_stencil_state_create_info.depthWriteEnable = pipeline_desc.depth_write_enable;
        pipeline_depth_stencil_state_create_info.depthCompareOp = GetNativeCompareOp(pipeline_desc.depth_compare_op);

        VkPipelineColorBlendAttachmentState pipeline_blend_attachment_states[MAX_COLOR_ATTACHMENT_COUNT] = {};
        for (uint32_t index = 0; index < pipeline_desc.color_format_count; ++index)
        {
            pipeline_blend_attachment_states[index].blendEnable = pipeline_desc.blend_enable;
            pipeline_blend_attachment_states[index].srcColorBlendFactor = GetNativeBlendFactor(pipeline_desc.src_blend_factor_color);
            pipeline_blend_attachment_states[index].dstColorBlendFactor = GetNativeBlendFactor(pipeline_desc.dst_blend_factor_color);
            pipeline_blend_attachment_states[index].colorBlendOp = GetNativeBlendOp(pipeline_desc.blend_op_color);
            pipeline_blend_attachment_states[index].srcAlphaBlendFactor = GetNativeBlendFactor(pipeline_desc.src_blend_factor_alpha);
            pipeline_blend_attachment_states[index].dstAlphaBlendFactor = GetNativeBlendFactor(pipeline_desc.dst_blend_factor_alpha);
            pipeline_blend_attachment_states[index].alphaBlendOp = GetNativeBlendOp(pipeline_desc.blend_op_alpha);
            pipeline_blend_attachment_states[index].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }

        VkPipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info = {};
        pipeline_color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipeline_color_blend_state_create_info.attachmentCount = pipeline_desc.color_format_count;
        pipeline_color_blend_state_create_info.pAttachments = pipeline_blend_attachment_states;

        VkFormat color_formats[MAX_COLOR_ATTACHMENT_COUNT];
        for (uint32_t index = 0; index < pipeline_desc.color_format_count; index++)
        {
            color_formats[index] = GetVkFormat(pipeline_desc.color_formats[index]);
        }

        VkPipelineRenderingCreateInfo pipeline_rendering_create_info = {};
        pipeline_rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipeline_rendering_create_info.colorAttachmentCount = pipeline_desc.color_format_count;
        pipeline_rendering_create_info.pColorAttachmentFormats = color_formats;
        pipeline_rendering_create_info.depthAttachmentFormat = GetVkFormat(pipeline_desc.depth_format);
           
        VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
        graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphics_pipeline_create_info.pNext = &pipeline_rendering_create_info;
        graphics_pipeline_create_info.stageCount = 2;
        graphics_pipeline_create_info.pStages = pipeline_shader_stage_create_info;
        graphics_pipeline_create_info.pVertexInputState = &pipeline_vertex_input_create_info;
        graphics_pipeline_create_info.pInputAssemblyState = &pipeline_input_assembly_create_info;
        graphics_pipeline_create_info.pViewportState = &pipeline_viewport_state_create_info;
        graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
        graphics_pipeline_create_info.pMultisampleState = &pipeline_multisample_state_create_info;
        graphics_pipeline_create_info.pDepthStencilState = &pipeline_depth_stencil_state_create_info;
        graphics_pipeline_create_info.pColorBlendState = &pipeline_color_blend_state_create_info;
        graphics_pipeline_create_info.pDynamicState = &pipeline_dynamic_state_create_info;
        graphics_pipeline_create_info.layout = g_pipeline_layout;

        VkResult result = RHI::vkCreateGraphicsPipelines(g_logical_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &graphicsPipeline->pipeline);
        ensure(result == VK_SUCCESS);

        return graphicsPipeline;
    }

    GPUComputePipeline* VulkanRHI::CreateComputePipeline(const GPUComputePipelineDesc& pipelineDesc)
    {
        VulkanGPUComputePipeline* computePipeline = new VulkanGPUComputePipeline();

        const VulkanGPUShader* computeShader = static_cast<const VulkanGPUShader*>(pipelineDesc.computeShader);

        VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {};
        pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        pipelineShaderStageCreateInfo.module = computeShader->module;
        pipelineShaderStageCreateInfo.pName = "CSMain";

        VkComputePipelineCreateInfo computePipelineCreateInfo = {};
        computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computePipelineCreateInfo.stage = pipelineShaderStageCreateInfo;
        computePipelineCreateInfo.layout = g_pipeline_layout;

        VkResult result = RHI::vkCreateComputePipelines(g_logical_device, VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &computePipeline->pipeline);
        ensure(result == VK_SUCCESS);

        return computePipeline;
    }
    
    GPUShader *VulkanRHI::CreateShader(const GPUShaderDesc &shader_desc)
    {
        VulkanGPUShader* native_shader = new VulkanGPUShader();

        VkShaderModuleCreateInfo shader_module_create_info = {};
        shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_module_create_info.codeSize = shader_desc.bytecode_size;
        shader_module_create_info.pCode = static_cast<const uint32_t*>(shader_desc.bytecode);

        VkResult result = RHI::vkCreateShaderModule(g_logical_device, &shader_module_create_info, nullptr, &native_shader->module);
        ensure(result == VK_SUCCESS);

        return native_shader;
    }
    
    GPUSwapchain *VulkanRHI::CreateSwapchain(const GPUSwapchainDesc &swapchain_desc)
    {
        VulkanGPUSwapchain* swapchain = new VulkanGPUSwapchain();

#if defined(_WIN32)
        CreateSurfaceWin32(g_instance, &swapchain->surface, swapchain_desc.window_platform.hwnd, GetModuleHandle(nullptr));
#elif defined(__linux__)
        CreateSurfaceX11(g_instance, &swapchain->surface, swapchain_desc.window_platform.display, reinterpret_cast<void*>(static_cast<uintptr_t>(swapchain_desc.window_platform.window)));
#endif

        VkSurfaceCapabilitiesKHR surface_capabilities;
        RHI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_physical_device, swapchain->surface, &surface_capabilities);

        swapchain->image_count = swapchain_desc.backbuffer_count;

        if (swapchain->image_count < surface_capabilities.minImageCount)
        {
            swapchain->image_count = surface_capabilities.minImageCount;
        }

        if (surface_capabilities.maxImageCount > 0 && swapchain->image_count > surface_capabilities.maxImageCount)
        {
            swapchain->image_count = surface_capabilities.maxImageCount;
        }

        if (surface_capabilities.currentExtent.width != UINT32_MAX)
        {
            swapchain->extent = surface_capabilities.currentExtent;
        }
        else
        {
            swapchain->extent.width = std::clamp(swapchain_desc.backbuffer_width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
            swapchain->extent.height = std::clamp(swapchain_desc.backbuffer_height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
        }

        uint32_t presentation = 0U;
        RHI::vkGetPhysicalDeviceSurfaceSupportKHR(g_physical_device, g_graphics_family, swapchain->surface, &presentation);
        ensure(presentation != 0U);

        uint32_t present_mode_count = 0;
        RHI::vkGetPhysicalDeviceSurfacePresentModesKHR(g_physical_device, swapchain->surface, &present_mode_count, nullptr);

        std::vector<VkPresentModeKHR> available_present_modes(present_mode_count);
        RHI::vkGetPhysicalDeviceSurfacePresentModesKHR(g_physical_device, swapchain->surface, &present_mode_count, available_present_modes.data());

        std::unordered_set<VkPresentModeKHR> supported_present_modes(available_present_modes.begin(), available_present_modes.end());
        VkPresentModeKHR desired_mode = GetNativePresentMode(swapchain_desc.present_mode);
        VkPresentModeKHR present_mode = supported_present_modes.contains(desired_mode) ? desired_mode : VK_PRESENT_MODE_FIFO_KHR;

        VkSwapchainCreateInfoKHR swapchain_create_info = {};
        swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchain_create_info.surface = swapchain->surface;
        swapchain_create_info.minImageCount = swapchain->image_count;
        swapchain_create_info.imageFormat = GetVkFormat(swapchain_desc.backbuffer_format);
        swapchain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        swapchain_create_info.imageExtent = swapchain->extent;
        swapchain_create_info.imageArrayLayers = 1;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.preTransform = surface_capabilities.currentTransform;
        swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchain_create_info.presentMode = present_mode;
        swapchain_create_info.clipped = VK_TRUE;

        VkResult result = RHI::vkCreateSwapchainKHR(g_logical_device, &swapchain_create_info, nullptr, &swapchain->swapchain);
        ensure(result == VK_SUCCESS);

        RHI::vkGetSwapchainImagesKHR(g_logical_device, swapchain->swapchain, &swapchain->image_count, nullptr);
        std::vector<VkImage> images(swapchain->image_count);
        RHI::vkGetSwapchainImagesKHR(g_logical_device, swapchain->swapchain, &swapchain->image_count, images.data());

        swapchain->image_available_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * swapchain->image_count);
        swapchain->render_finished_semaphores = (VkSemaphore*)malloc(sizeof(VkSemaphore) * swapchain->image_count);

        for (uint32_t index = 0; index < swapchain->image_count; ++index)
        {
            swapchain->textures[index] = new VulkanGPUTexture();
            swapchain->textures[index]->texture = images[index];
            swapchain->textures[index]->allocation = nullptr;

            VkImageViewCreateInfo image_view_create_info = {};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = images[index];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = GetVkFormat(swapchain_desc.backbuffer_format);
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;
            
            RHI::vkCreateImageView(g_logical_device, &image_view_create_info, nullptr, &swapchain->textures[index]->rtv);

            VkSemaphoreCreateInfo semaphore_create_info = {};
            semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            RHI::vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &swapchain->image_available_semaphores[index]);
            RHI::vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &swapchain->render_finished_semaphores[index]);
        }

        std::printf("Swapchain:\n\tImage Count: %d\n\tFrame Count: %d\n", swapchain->image_count, CONCURRENT_FRAME_COUNT);
        
        return swapchain;
    }
    
    GPUCommandList *VulkanRHI::CreateCommandList(const GPUCommandListDesc& desc)
    {
        VulkanGPUCommandList* command_list = new VulkanGPUCommandList();

        for (uint32_t index = 0; index < CONCURRENT_FRAME_COUNT; ++index)
        {
            VkCommandPoolCreateInfo command_pool_create_info = {};
            command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

            command_pool_create_info.queueFamilyIndex = GetQueueFamily(desc.queue_flags);

            VkResult result = RHI::vkCreateCommandPool(g_logical_device, &command_pool_create_info, nullptr, &command_list->command_pool_data[index]);
            ensure(result == VK_SUCCESS);

            VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
            command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            command_buffer_allocate_info.commandPool = command_list->command_pool_data[index];
            command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            command_buffer_allocate_info.commandBufferCount = 1;

            result = RHI::vkAllocateCommandBuffers(g_logical_device, &command_buffer_allocate_info, &command_list->command_buffer_data[index]);
            ensure(result == VK_SUCCESS);
        }

        command_list->queue_flags = desc.queue_flags;

        return command_list;
    }

    GPUFence* VulkanRHI::CreateFence()
    {
        VulkanGPUFence* fence = new VulkanGPUFence();

        VkSemaphoreTypeCreateInfo semaphore_type_create_info = {};
        semaphore_type_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
        semaphore_type_create_info.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
        semaphore_type_create_info.initialValue = 0;

        VkSemaphoreCreateInfo semaphore_create_info = {};
        semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_create_info.pNext = &semaphore_type_create_info;

        VkResult result = RHI::vkCreateSemaphore(g_logical_device, &semaphore_create_info, nullptr, &fence->semaphore);
        ensure(result == VK_SUCCESS);

        return fence;
    }
    
    void VulkanRHI::SetTexture(uint32_t index, const GPUTexture* texture)
    {
        const VulkanGPUTexture* vulkan_texture = static_cast<const VulkanGPUTexture*>(texture);

        // Textures bind in pairs: the regular view at the even slot and the sRGB view
        // right after it, which is the regular view again for a texture without one.
        VkDescriptorImageInfo descriptor_texture_info[2] = {};
        descriptor_texture_info[0].imageView = vulkan_texture->srv;
        descriptor_texture_info[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        descriptor_texture_info[1].imageView = vulkan_texture->srgb_srv != nullptr ? vulkan_texture->srgb_srv : vulkan_texture->srv;
        descriptor_texture_info[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write_descriptor_set = {};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = g_descriptor_set;
        write_descriptor_set.dstBinding = BINDING_SAMPLED_IMAGES;
        write_descriptor_set.dstArrayElement = index - index % 2;
        write_descriptor_set.descriptorCount = 2;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write_descriptor_set.pImageInfo = descriptor_texture_info;

        RHI::vkUpdateDescriptorSets(g_logical_device, 1, &write_descriptor_set, 0, nullptr);
    }

    void VulkanRHI::SetRWTexture(uint32_t index, const GPUTexture* texture, uint32_t mip)
    {
        VkDescriptorImageInfo descriptor_texture_info = {};
        descriptor_texture_info.imageView = static_cast<const VulkanGPUTexture*>(texture)->uavs[mip];
        descriptor_texture_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        VkWriteDescriptorSet write_descriptor_set = {};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = g_descriptor_set;
        write_descriptor_set.dstBinding = BINDING_STORAGE_IMAGES;
        write_descriptor_set.dstArrayElement = index;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        write_descriptor_set.pImageInfo = &descriptor_texture_info;

        RHI::vkUpdateDescriptorSets(g_logical_device, 1, &write_descriptor_set, 0, nullptr);
    }
    
    void VulkanRHI::SetBuffer(uint32_t index, const GPUBuffer* buffer)
    {
        VkDescriptorBufferInfo descriptor_buffer_info = {};
        descriptor_buffer_info.buffer = static_cast<const VulkanGPUBuffer*>(buffer)->buffer;
        descriptor_buffer_info.offset = 0;
        descriptor_buffer_info.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet write_descriptor_set = {};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = g_descriptor_set;
        write_descriptor_set.dstBinding = BINDING_STORAGE_BUFFERS;
        write_descriptor_set.dstArrayElement = index;
        write_descriptor_set.descriptorCount = 1;
        write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        write_descriptor_set.pBufferInfo = &descriptor_buffer_info;

        RHI::vkUpdateDescriptorSets(g_logical_device, 1, &write_descriptor_set, 0, nullptr);
    }

    void VulkanRHI::SetSampler(uint32_t index, const GPUSampler* sampler)
    {
        VkDescriptorImageInfo descriptorImageInfo = {};
        descriptorImageInfo.sampler = static_cast<const VulkanGPUSampler*>(sampler)->sampler;
        
        VkWriteDescriptorSet writeDescriptorSet = {};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.dstSet = g_descriptor_set;
        writeDescriptorSet.dstBinding = BINDING_SAMPLERS;
        writeDescriptorSet.dstArrayElement = index;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writeDescriptorSet.pImageInfo = &descriptorImageInfo;

        RHI::vkUpdateDescriptorSets(g_logical_device, 1, &writeDescriptorSet, 0, nullptr);
    }

    VulkanGPUCommandList::~VulkanGPUCommandList()
    {
        for (uint32_t index = 0; index < CONCURRENT_FRAME_COUNT; ++index)
        {
            RHI::vkDestroyCommandPool(g_logical_device, command_pool_data[index], nullptr);
        }
    }

    void VulkanGPUCommandList::Begin()
    {
        RHI::vkResetCommandPool(g_logical_device, command_pool_data[CURRENT_FRAME], 0);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        RHI::vkBeginCommandBuffer(command_buffer_data[CURRENT_FRAME], &begin_info);

        // A transfer queue only copies. It cannot bind descriptor sets, and the
        // frame's queries are not its to reset.
        if (queue_flags == GPUQueueFlag::Transfer)
        {
            return;
        }

        RHI::vkCmdBindDescriptorSets(command_buffer_data[CURRENT_FRAME], VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline_layout, 0, 1, &g_descriptor_set, 0, nullptr);
        RHI::vkCmdBindDescriptorSets(command_buffer_data[CURRENT_FRAME], VK_PIPELINE_BIND_POINT_COMPUTE, g_pipeline_layout, 0, 1, &g_descriptor_set, 0, nullptr);

        RHI::vkCmdResetQueryPool(command_buffer_data[CURRENT_FRAME], g_timeline_query_pool_data[CURRENT_FRAME], 0, MAX_TIMESTAMP_COUNT);
        RHI::vkCmdResetQueryPool(command_buffer_data[CURRENT_FRAME], g_pipeline_query_pool_data[CURRENT_FRAME], 0, 1);
        g_query_pool_counter[CURRENT_FRAME] = 0;
    }

    void VulkanGPUCommandList::End()
    {
        RHI::vkEndCommandBuffer(command_buffer_data[CURRENT_FRAME]);
    }
    
    void VulkanGPUCommandList::BeginRenderPass(GPURenderPass* render_pass)  
    {
        VkRenderingAttachmentInfo colorAttachmentInfoData[MAX_COLOR_ATTACHMENT_COUNT] = {};
        for (uint32_t index = 0; index < render_pass->color_attachment_count; ++index)
        {
            const GPUColorAttachment* colorAttachment = render_pass->color_attachments[index];

            colorAttachmentInfoData[index].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            colorAttachmentInfoData[index].imageView = static_cast<const VulkanGPUTexture*>(colorAttachment->M_Texture)->rtv;
            colorAttachmentInfoData[index].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachmentInfoData[index].loadOp = colorAttachment->loadOp == GPULoadOp::Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : colorAttachment->loadOp == GPULoadOp::Load ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachmentInfoData[index].storeOp = colorAttachment->storeOp == GPUStoreOp::Store ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            memcpy(&colorAttachmentInfoData[index].clearValue.color.float32, &colorAttachment->M_Clear_Value.color, sizeof(float) * 4);
        }

        VkRenderingAttachmentInfo depthAttachmentInfo = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO };
        if (render_pass->depth_attachment)
        {
            const GPUDepthAttachment* depthAttachment = render_pass->depth_attachment;

            depthAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            depthAttachmentInfo.imageView = static_cast<const VulkanGPUTexture*>(depthAttachment->texture)->dsv;
            depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            depthAttachmentInfo.loadOp = depthAttachment->load_op == GPULoadOp::Clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : depthAttachment->load_op == GPULoadOp::Load ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachmentInfo.storeOp = depthAttachment->store_op == GPUStoreOp::Store ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
            memcpy(&depthAttachmentInfo.clearValue.depthStencil, &depthAttachment->clear_value.depth_stencil, sizeof(VkClearDepthStencilValue));
        }

        VkRenderingInfo rendering_info = {};
        rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        rendering_info.renderArea = { {0, 0}, { render_pass->render_width, render_pass->render_height } };
        rendering_info.layerCount = 1;
        rendering_info.colorAttachmentCount = render_pass->color_attachment_count;
        rendering_info.pColorAttachments = colorAttachmentInfoData;
        rendering_info.pDepthAttachment = render_pass->depth_attachment ? &depthAttachmentInfo : nullptr;

        RHI::vkCmdBeginRendering(command_buffer_data[CURRENT_FRAME], &rendering_info);
    }
    
    void VulkanGPUCommandList::EndRenderPass()
    {
        RHI::vkCmdEndRendering(command_buffer_data[CURRENT_FRAME]);
    }
    
    void VulkanGPUCommandList::SetGraphicsPipeline(const GPUGraphicsPipeline* pipeline)
    {
        RHI::vkCmdBindPipeline(command_buffer_data[CURRENT_FRAME], VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<const VulkanGPUGraphicsPipeline*>(pipeline)->pipeline);
    }

    void VulkanGPUCommandList::SetComputePipeline(const GPUComputePipeline* computePipeline)
    {
        RHI::vkCmdBindPipeline(command_buffer_data[CURRENT_FRAME], VK_PIPELINE_BIND_POINT_COMPUTE, static_cast<const VulkanGPUComputePipeline*>(computePipeline)->pipeline);
    }
    
    void VulkanGPUCommandList::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
    {
        VkViewport viewport_data[] = { { x, y, width, height, min_depth, max_depth } };
        RHI::vkCmdSetViewport(command_buffer_data[CURRENT_FRAME], 0, std::size(viewport_data), viewport_data);
    }
    
    void VulkanGPUCommandList::SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        VkRect2D scissor_data[] = { { { x, y }, { width, height } } };
        RHI::vkCmdSetScissor(command_buffer_data[CURRENT_FRAME], 0, std::size(scissor_data), scissor_data);
    }
    
    void VulkanGPUCommandList::SetPushConstants(const void* data, uint32_t size)
    {
        RHI::vkCmdPushConstants(command_buffer_data[CURRENT_FRAME], g_pipeline_layout, VK_SHADER_STAGE_ALL, 0, size, data);
    }
    
    void VulkanGPUCommandList::SetIndexBuffer(const GPUBuffer* buffer, uint64_t offset)
    {
        RHI::vkCmdBindIndexBuffer(command_buffer_data[CURRENT_FRAME], static_cast<const VulkanGPUBuffer*>(buffer)->buffer, offset, VK_INDEX_TYPE_UINT32);
    }

    void VulkanGPUCommandList::Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance)
    {
        RHI::vkCmdDraw(command_buffer_data[CURRENT_FRAME], vertex_count, instance_count, first_vertex, first_instance);
    }
    
    void VulkanGPUCommandList::DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance)
    {
        RHI::vkCmdDrawIndexed(command_buffer_data[CURRENT_FRAME], index_count, instance_count, first_index, vertex_offset, first_instance);
    }
    
    void VulkanGPUCommandList::DrawIndirect(const GPUBuffer* buffer, uint64_t offset, uint32_t draw_count)
    {
        RHI::vkCmdDrawIndirect(command_buffer_data[CURRENT_FRAME], static_cast<const VulkanGPUBuffer*>(buffer)->buffer, offset, draw_count, sizeof(VkDrawIndirectCommand));
    }

    void VulkanGPUCommandList::DrawIndexedIndirectCount(const GPUBuffer* buffer, uint64_t offset, const GPUBuffer* count_buffer, uint64_t count_offset, uint32_t max_draw_count)
    {
        RHI::vkCmdDrawIndexedIndirectCount(command_buffer_data[CURRENT_FRAME], static_cast<const VulkanGPUBuffer*>(buffer)->buffer, offset, static_cast<const VulkanGPUBuffer*>(count_buffer)->buffer, count_offset, max_draw_count, sizeof(VkDrawIndexedIndirectCommand));
    }

    void VulkanGPUCommandList::Dispatch(uint32_t x, uint32_t y, uint32_t z)
    {
        RHI::vkCmdDispatch(command_buffer_data[CURRENT_FRAME], x, y, z);
    }

    void VulkanGPUCommandList::FillBuffer(GPUBuffer* buffer, uint64_t size, uint64_t offset, uint32_t value)
    {
        RHI::vkCmdFillBuffer(command_buffer_data[CURRENT_FRAME], static_cast<const VulkanGPUBuffer*>(buffer)->buffer, offset, size, value);
    }

    void VulkanGPUCommandList::CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, uint64_t size, uint64_t src_offset, uint64_t dst_offset)
    {
        VkBufferCopy2 region = {};
        region.sType = VK_STRUCTURE_TYPE_BUFFER_COPY_2;
        region.srcOffset = src_offset;
        region.dstOffset = dst_offset;
        region.size = size;

        VkCopyBufferInfo2 copy_info = {};
        copy_info.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2;
        copy_info.srcBuffer = static_cast<const VulkanGPUBuffer*>(src)->buffer;
        copy_info.dstBuffer = static_cast<const VulkanGPUBuffer*>(dst)->buffer;
        copy_info.regionCount = 1;
        copy_info.pRegions = &region;

        RHI::vkCmdCopyBuffer2(command_buffer_data[CURRENT_FRAME], &copy_info);
    }

    void VulkanGPUCommandList::CopyBufferToTexture(GPUBuffer* src, uint64_t src_offset, GPUTexture* dst, uint32_t width, uint32_t height, uint32_t mip, uint32_t layer)
    {
        VkBufferImageCopy2 region = {};
        region.sType = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2;
        region.bufferOffset = src_offset;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mip;
        region.imageSubresource.baseArrayLayer = layer;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        VkCopyBufferToImageInfo2 copyInfo = {};
        copyInfo.sType = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2;
        copyInfo.srcBuffer = static_cast<const VulkanGPUBuffer*>(src)->buffer;
        copyInfo.dstImage = static_cast<const VulkanGPUTexture*>(dst)->texture;
        copyInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copyInfo.regionCount = 1;
        copyInfo.pRegions = &region;

        RHI::vkCmdCopyBufferToImage2(command_buffer_data[CURRENT_FRAME], &copyInfo);
    }

    void VulkanGPUCommandList::CopyTextureToTexture(GPUTexture* src, GPUTexture* dst, uint32_t width, uint32_t height)
    {
        VkImageBlit2 region = {};
        region.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2;
        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.layerCount = 1;
        region.srcOffsets[1] = { (int32_t)width, (int32_t)height, 1 };
        region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.dstSubresource.layerCount = 1;
        region.dstOffsets[1] = { (int32_t)width, (int32_t)height, 1 };

        VkBlitImageInfo2 blit_info = {};
        blit_info.sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2;
        blit_info.srcImage = static_cast<const VulkanGPUTexture*>(src)->texture;
        blit_info.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        blit_info.dstImage = static_cast<const VulkanGPUTexture*>(dst)->texture;
        blit_info.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        blit_info.regionCount = 1;
        blit_info.pRegions = &region;
        blit_info.filter = VK_FILTER_LINEAR;

        RHI::vkCmdBlitImage2(command_buffer_data[CURRENT_FRAME], &blit_info);
    }

    void VulkanGPUCommandList::Barrier(const GPUBufferBarrier* buffer_barriers, uint32_t buffer_barrier_count, const GPUTextureBarrier* texture_barriers, uint32_t texture_barrier_count)
    {
        VkBufferMemoryBarrier2 buffer_barrier_data[MAX_BARRIER_COUNT];
        memset(buffer_barrier_data, 0, sizeof(VkBufferMemoryBarrier2) * buffer_barrier_count);
        uint32_t buffer_barrier_data_count = 0;
        for (uint32_t index = 0; index < buffer_barrier_count; ++index)
        {
            const VulkanGPUBuffer* native_buffer = static_cast<const VulkanGPUBuffer*>(buffer_barriers[index].buffer);

            VkPipelineStageFlags2 src_stage_flags;
            VkPipelineStageFlags2 dst_stage_flags;
            VkAccessFlags2 src_access_flags;
            VkAccessFlags2 dst_access_flags;

            GetBarrierInfo(buffer_barriers[index].src_access, src_stage_flags, src_access_flags);
            GetBarrierInfo(buffer_barriers[index].dst_access, dst_stage_flags, dst_access_flags);

            if (!GetQueueTransferInfo(queue_flags, buffer_barriers[index].src_queue, buffer_barriers[index].dst_queue, src_access_flags, dst_stage_flags, dst_access_flags))
            {
                continue;
            }

            VkBufferMemoryBarrier2& buffer_barrier = buffer_barrier_data[buffer_barrier_data_count++];
            buffer_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
            buffer_barrier.srcStageMask = src_stage_flags;
            buffer_barrier.srcAccessMask = src_access_flags;
            buffer_barrier.dstStageMask = dst_stage_flags;
            buffer_barrier.dstAccessMask = dst_access_flags;
            buffer_barrier.srcQueueFamilyIndex = GetQueueFamily(buffer_barriers[index].src_queue);
            buffer_barrier.dstQueueFamilyIndex = GetQueueFamily(buffer_barriers[index].dst_queue);
            buffer_barrier.buffer = native_buffer->buffer;
            buffer_barrier.offset = 0;
            buffer_barrier.size = VK_WHOLE_SIZE;
        }

        VkImageMemoryBarrier2 texture_barrier_data[MAX_BARRIER_COUNT];
        memset(texture_barrier_data, 0, sizeof(VkImageMemoryBarrier2) * texture_barrier_count);
        uint32_t texture_barrier_data_count = 0;
        for (uint32_t index = 0; index < texture_barrier_count; ++index)
        {
            const VulkanGPUTexture* native = static_cast<const VulkanGPUTexture*>(texture_barriers[index].texture);

            VkImageLayout old_layout;
            VkImageLayout new_layout;
            VkPipelineStageFlags2 src_stage;
            VkPipelineStageFlags2 dst_stage;
            VkAccessFlags2 src_access;
            VkAccessFlags2 dst_access;
            
            GetBarrierInfo(texture_barriers[index].old_layout, old_layout, src_stage, src_access);
            GetBarrierInfo(texture_barriers[index].new_layout, new_layout, dst_stage, dst_access);

            if (!GetQueueTransferInfo(queue_flags, texture_barriers[index].src_queue, texture_barriers[index].dst_queue, src_access, dst_stage, dst_access))
            {
                continue;
            }

            VkImageMemoryBarrier2& texture_barrier = texture_barrier_data[texture_barrier_data_count++];
            texture_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
            texture_barrier.oldLayout = old_layout;
            texture_barrier.newLayout = new_layout;
            texture_barrier.srcStageMask = src_stage;
            texture_barrier.srcAccessMask = src_access;
            texture_barrier.dstStageMask = dst_stage;
            texture_barrier.dstAccessMask = dst_access;
            texture_barrier.srcQueueFamilyIndex = GetQueueFamily(texture_barriers[index].src_queue);
            texture_barrier.dstQueueFamilyIndex = GetQueueFamily(texture_barriers[index].dst_queue);
            texture_barrier.image = native->texture;
            texture_barrier.subresourceRange.aspectMask = (old_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL || new_layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            texture_barrier.subresourceRange.baseMipLevel = 0;
            texture_barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
            texture_barrier.subresourceRange.baseArrayLayer = 0;
            texture_barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
        }

        VkDependencyInfo dependency_info = {};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.imageMemoryBarrierCount = texture_barrier_data_count;
        dependency_info.pImageMemoryBarriers = texture_barrier_data;
        dependency_info.bufferMemoryBarrierCount = buffer_barrier_data_count;
        dependency_info.pBufferMemoryBarriers = buffer_barrier_data;

        RHI::vkCmdPipelineBarrier2(command_buffer_data[CURRENT_FRAME], &dependency_info);
    }

    uint32_t VulkanGPUCommandList::BeginTimelineQuery()
    {
        uint32_t index = g_query_pool_counter[CURRENT_FRAME]++;
        ensure(index < MAX_TIMESTAMP_COUNT);
        // Not top of pipe, which does not wait for the work before it and so counts it.
        VkPipelineStageFlagBits pipeline_stage_flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        RHI::vkCmdWriteTimestamp(command_buffer_data[CURRENT_FRAME], pipeline_stage_flag, g_timeline_query_pool_data[CURRENT_FRAME], index);

        return index;
    }

    uint32_t VulkanGPUCommandList::EndTimelineQuery()
    {
        uint32_t index = g_query_pool_counter[CURRENT_FRAME]++;
        ensure(index < MAX_TIMESTAMP_COUNT);
        VkPipelineStageFlagBits pipeline_stage_flag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        RHI::vkCmdWriteTimestamp(command_buffer_data[CURRENT_FRAME], pipeline_stage_flag, g_timeline_query_pool_data[CURRENT_FRAME], index);

        return index;
    }

    void VulkanGPUCommandList::BeginPipelineQuery()
    {
        RHI::vkCmdBeginQuery(command_buffer_data[CURRENT_FRAME], g_pipeline_query_pool_data[CURRENT_FRAME], 0, 0);
    }
    
    void VulkanGPUCommandList::EndPipelineQuery()
    {
        RHI::vkCmdEndQuery(command_buffer_data[CURRENT_FRAME], g_pipeline_query_pool_data[CURRENT_FRAME], 0);
    }
}
