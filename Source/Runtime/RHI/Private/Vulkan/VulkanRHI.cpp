#include "VulkanRHI.h"
#include <vector>
#include <assert.h>
#include <unordered_set>
#include <string_view>

RHI::IRHI* CreateVulkanRHI()
{
    return new RHI::VulkanRHI();
}

namespace RHI
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL ExecDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
    {
        printf("[Vulkan] %s\n", CallbackData->pMessage);
        return VK_FALSE;
    }

    static VkInstance           g_instance          = nullptr;
    static VkPhysicalDevice     g_physical_device   = nullptr;
    static VkDevice             g_logical_device    = nullptr;

    static uint32_t             g_graphics_family   = UINT32_MAX;
    static uint32_t             g_compute_family    = UINT32_MAX;
    static uint32_t             g_transfer_family   = UINT32_MAX;

    static GPUAdapter           g_adapter           = {};

    static void SetDebugName(VkObjectType type, uint64_t handle, const char* debug_name)
    {
        VkDebugUtilsObjectNameInfoEXT debug_info = {};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debug_info.objectType = type;
        debug_info.objectHandle = handle;
        debug_info.pObjectName = debug_name;
    }

    VulkanGPUBuffer::VulkanGPUBuffer(VkDevice in_device)
        : device(in_device)
    {
    }

    void VulkanGPUBuffer::SetDebugName(const char *debug_name)
    {
    }
    
    uint64_t VulkanGPUBuffer::GetVirtualAddress() const
    {
        VkBufferDeviceAddressInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO ;
        info.buffer = buffer;
        return RHI::vkGetBufferDeviceAddress(device, &info);
    }

    VulkanRHI::VulkanRHI()
    {
        RHI::LoadVulkanLibrary();

        VkApplicationInfo ApplicationInfo = {};
        ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        ApplicationInfo.pApplicationName = "Rocket";
        ApplicationInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        ApplicationInfo.pEngineName = "No Engine";
        ApplicationInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
        ApplicationInfo.apiVersion = VK_API_VERSION_1_3;

        VkDebugUtilsMessengerCreateInfoEXT validation_callback_create_info = {};
        validation_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        validation_callback_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        validation_callback_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        validation_callback_create_info.pfnUserCallback = &ExecDebugCallback;

        std::vector<const char*> instance_extensions =
        {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME
        };

        std::vector<const char*> validation_extensions = 
        {
            "VK_LAYER_KHRONOS_validation"
        };


        std::vector<const char*> physical_device_extensions =
        {
            "VK_KHR_swapchain"
        };

        VkInstanceCreateInfo instance_create_info = {};
        instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instance_create_info.pApplicationInfo = &ApplicationInfo;
        instance_create_info.ppEnabledExtensionNames = instance_extensions.data();
        instance_create_info.enabledExtensionCount = instance_extensions.size();
        instance_create_info.ppEnabledLayerNames = validation_extensions.data();
        instance_create_info.enabledLayerCount = validation_extensions.size();
        instance_create_info.pNext = &validation_callback_create_info;

        RHI::LoadVulkanGlobals();

        VkResult result = RHI::vkCreateInstance(&instance_create_info, nullptr, &g_instance);
        assert(result == VK_SUCCESS);

        RHI::LoadVulkanInstance(g_instance);

        uint32_t DeviceCount = 0;
        RHI::vkEnumeratePhysicalDevices(g_instance, &DeviceCount, nullptr);

        std::vector<VkPhysicalDevice> physical_devices(DeviceCount);
        RHI::vkEnumeratePhysicalDevices(g_instance, &DeviceCount, physical_devices.data());

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
            uint32_t ExtensionCount;
            RHI::vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ExtensionCount, nullptr);

            std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
            RHI::vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &ExtensionCount, AvailableExtensions.data());

            std::unordered_set<std::string_view> Filter;
            Filter.insert(physical_device_extensions.begin(), physical_device_extensions.end());

            for (const VkExtensionProperties& Extension : AvailableExtensions)
            {
                if (Filter.contains(Extension.extensionName))
                {
                    Filter.erase(Extension.extensionName);
                }
            }

            if (Filter.empty())
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

        VkPhysicalDeviceVulkan13Features request_device_features_1_3 = {};
        request_device_features_1_3.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
        request_device_features_1_3.pNext = &request_device_features_1_2;
        request_device_features_1_3.dynamicRendering = VK_TRUE;
        request_device_features_1_3.synchronization2 = VK_TRUE;

        VkPhysicalDeviceFeatures request_device_features = {};
        request_device_features.shaderInt64 = VK_TRUE;
        request_device_features.samplerAnisotropy = VK_TRUE;
        request_device_features.multiDrawIndirect = VK_TRUE;

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

        std::unordered_set<uint32_t> unique_families = { g_graphics_family, g_compute_family, g_transfer_family };
        
        for (uint32_t family : unique_families)
        {
            queue_create_infos[queue_create_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_infos[queue_create_count].queueFamilyIndex = family;
            queue_create_infos[queue_create_count].queueCount = 1;
            queue_create_infos[queue_create_count].pQueuePriorities = &queue_priority;
            queue_create_count++;
        }

        VkDeviceCreateInfo DeviceCreateInfo = {};
        DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        DeviceCreateInfo.pQueueCreateInfos = queue_create_infos;
        DeviceCreateInfo.queueCreateInfoCount = queue_create_count;
        DeviceCreateInfo.pEnabledFeatures = &request_device_features;
        DeviceCreateInfo.pNext = &request_device_features_1_3;
        DeviceCreateInfo.ppEnabledExtensionNames = physical_device_extensions.data();
        DeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(physical_device_extensions.size());
        DeviceCreateInfo.ppEnabledLayerNames = validation_extensions.data();
        DeviceCreateInfo.enabledLayerCount = validation_extensions.size();

        VkResult Result = RHI::vkCreateDevice(g_physical_device, &DeviceCreateInfo, nullptr, &g_logical_device);
        assert(Result == VK_SUCCESS);

        RHI::LoadVulkanDevice(g_logical_device);
    }

    VulkanRHI::~VulkanRHI()
    {
    }

    GPUAdapter VulkanRHI::GetAdapater() const
    {
        return GPUAdapter();
    }
    
    void VulkanRHI::Submit(GPUCommandList *command_list)
    {
    }
    
    void VulkanRHI::WaitIdle()
    {
    }
    
    GPUBuffer *VulkanRHI::CreateBuffer(const GPUBufferDesc &buffer_desc)
    {
        return nullptr;
    }
    
    GPUTexture *VulkanRHI::CreateTexture(const GPUTextureDesc &texture_desc)
    {
        return nullptr;
    }
    
    GPUSampler *VulkanRHI::CreateSampler(const GPUSamplerDesc &sampler_desc)
    {
        return nullptr;
    }
    
    GPUBindGroup *VulkanRHI::CreateBindGroup(const GPUBindGroupDesc &bind_group_desc)
    {
        return nullptr;
    }
    
    GPUPipeline *VulkanRHI::CreateGraphicsPipeline(const GPUGraphicsPipelineDesc &pipeline_desc)
    {
        return nullptr;
    }
    
    GPUShader *VulkanRHI::CreateShader(const GPUShaderDesc &pipeline_desc)
    {
        return nullptr;
    }
    
    GPUSwapchain *VulkanRHI::CreateSwapchain(const GPUSwapchainDesc &desc)
    {
        return nullptr;
    }
    
    GPUCommandList *VulkanRHI::CreateCommandList()
    {
        return nullptr;
    }
    
    void VulkanRHI::DestroyBuffer(GPUBuffer *buffer)
    {
    }
    
    void VulkanRHI::DestroyTexture(GPUTexture *texture)
    {
    }
    
    void VulkanRHI::DestroySampler(GPUSampler *sampler)
    {
    }
    
    void VulkanRHI::DestroyShader(GPUShader *shader)
    {
    }
    
    void VulkanRHI::DestroyPipeline(GPUPipeline *pipeline)
    {
    }
    
    void VulkanRHI::DestroyBindGroup(GPUBindGroup *bind_group)
    {
    }
}