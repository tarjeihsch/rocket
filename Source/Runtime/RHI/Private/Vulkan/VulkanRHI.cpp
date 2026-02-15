#include "VulkanRHI.h"

#include <vector>
#include <assert.h>
#include <unordered_set>
#include <string_view>
#include <algorithm>

#include "VulkanTypes.h"
#include <unordered_map>

RHI::IRHI* CreateVulkanRHI()
{
    return new RHI::VulkanRHI();
}

namespace RHI
{
    static constexpr uint32_t MAX_BIND_GROUP_ENTRIES = 16;

    static VKAPI_ATTR VkBool32 VKAPI_CALL ExecDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData)
    {
        printf("[Vulkan] %s\n", CallbackData->pMessage);
        return VK_FALSE;
    }

    static VkInstance           g_instance          = nullptr;
    static VkPhysicalDevice     g_physical_device   = nullptr;
    static VkDevice             g_logical_device    = nullptr;
    static VmaAllocator         g_allocator         = nullptr;

    static uint32_t             g_graphics_family   = UINT32_MAX;
    static uint32_t             g_compute_family    = UINT32_MAX;
    static uint32_t             g_transfer_family   = UINT32_MAX;

    static GPUAdapter           g_adapter           = {};

    static void SetDebugNameInternal(VkObjectType type, uint64_t handle, const char* debug_name)
    {
        VkDebugUtilsObjectNameInfoEXT debug_info = {};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        debug_info.objectType = type;
        debug_info.objectHandle = handle;
        debug_info.pObjectName = debug_name;
        RHI::vkSetDebugUtilsObjectNameEXT(g_logical_device, &debug_info);
    }

    VulkanGPUBuffer::~VulkanGPUBuffer()
    {
        vmaDestroyBuffer(g_allocator, buffer, allocation);
    }

    void VulkanGPUBuffer::SetDebugName(const char *debug_name)
    {
        SetDebugNameInternal(VK_OBJECT_TYPE_BUFFER, reinterpret_cast<uint64_t>(buffer), debug_name);
    }

    VulkanGPUTexture::~VulkanGPUTexture()
    {
        vmaDestroyImage(g_allocator, texture, allocation);
        
        for (VkImageView view : views)
        {
            vkDestroyImageView(g_logical_device, view, nullptr);
        }
    }

    void VulkanGPUTexture::SetDebugName(const char* debug_name)
    {
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
        void* mapped = nullptr;
        VkResult result = vmaMapMemory(g_allocator, allocation, &mapped);
        assert(result == VK_SUCCESS);
        return mapped;
    }

    void VulkanGPUBuffer::Unlock()
    {
        vmaUnmapMemory(g_allocator, allocation);
    }

    VulkanGPUShaderResources::~VulkanGPUShaderResources()
    {
        RHI::vkDestroyDescriptorPool(g_logical_device, descriptor_pool, nullptr);
        RHI::vkDestroyDescriptorSetLayout(g_logical_device, descriptor_set_layout, nullptr);
    }

    void VulkanGPUShaderResources::SetDebugName(const char* debug_name)
    {
    }

    void VulkanGPUShaderResources::SetBuffer(const GPUBuffer* buffer, uint32_t position, uint32_t index)
    {

    }

    void VulkanGPUShaderResources::SetTexture(const GPUTexture* texture, uint32_t position, uint32_t index)
    {
        const VulkanGPUTexture* native_texture = static_cast<const VulkanGPUTexture*>(texture);

        VkDescriptorImageInfo image_info = {};
        image_info.imageView = native_texture->srv;
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptor_set;
        write.dstBinding = position;
        write.dstArrayElement = index;
        write.descriptorCount = 1;
        write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        write.pImageInfo = &image_info;

        RHI::vkUpdateDescriptorSets(g_logical_device, 1, &write, 0, nullptr);
    }

    void VulkanGPUShaderResources::SetSampler(const GPUSampler* sampler, uint32_t position, uint32_t index)
    {
    }

    VulkanGPUShader::~VulkanGPUShader()
    {
        RHI::vkDestroyShaderModule(g_logical_device, module, nullptr);
    }

    VulkanGPUGraphicsPipeline::~VulkanGPUGraphicsPipeline()
    {
        RHI::vkDestroyPipeline(g_logical_device, pipeline, nullptr);
        RHI::vkDestroyPipelineLayout(g_logical_device, layout, nullptr);
    }

    void VulkanGPUGraphicsPipeline::SetDebugName(const char* debug_name)
    {
        
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

        VmaAllocatorCreateInfo AllocatorCreateInfo = {};
        AllocatorCreateInfo.physicalDevice = g_physical_device;
        AllocatorCreateInfo.device = g_logical_device;
        AllocatorCreateInfo.instance = g_instance;
        AllocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        AllocatorCreateInfo.pVulkanFunctions = &allocator_functions;

        Result = vmaCreateAllocator(&AllocatorCreateInfo, &g_allocator);
        assert(Result == VK_SUCCESS);
    }

    VulkanRHI::~VulkanRHI()
    {
        RHI::vkDeviceWaitIdle(g_logical_device);

        vmaDestroyAllocator(g_allocator);

        RHI::vkDestroyDevice(g_logical_device, nullptr);
        RHI::vkDestroyInstance(g_instance, nullptr);
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
        RHI::vkDeviceWaitIdle(g_logical_device);
    }
    
    GPUBuffer *VulkanRHI::CreateBuffer(const GPUBufferDesc &buffer_desc)
    {
        VulkanGPUBuffer* buffer = new VulkanGPUBuffer();
        
        VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Vertex) buffer_usage_flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Index) buffer_usage_flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Storage) buffer_usage_flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        if (buffer_desc.flags & GPUBufferFlag::Uniform) buffer_usage_flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

        VmaAllocationCreateFlags allocation_flags = 0;
        switch (buffer_desc.heap_type)
        {
            case GPUHeapType::Upload: allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; break;
            case GPUHeapType::Readback: allocation_flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT; break;
        }

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = buffer_desc.size;
        buffer_info.usage = buffer_usage_flags;

        VmaAllocationCreateInfo allocation_info = {};
        allocation_info.usage = VMA_MEMORY_USAGE_AUTO;
        allocation_info.flags = allocation_flags;

        VkResult result = vmaCreateBuffer(g_allocator, &buffer_info, &allocation_info, &buffer->buffer, &buffer->allocation, nullptr);
        assert(result == VK_SUCCESS);

        return buffer;
    }
    
    GPUTexture* VulkanRHI::CreateTexture(const GPUTextureDesc& texture_desc)
    {
        VulkanGPUTexture* texture = new VulkanGPUTexture();

        VkFormat format = GetNativeFormat(texture_desc.format);

        VkImageUsageFlags image_usage_flags = 0;
        if (texture_desc.flags & GPUTextureFlag::RenderTarget) image_usage_flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if (texture_desc.flags & GPUTextureFlag::DepthTarget) image_usage_flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        if (texture_desc.flags & GPUTextureFlag::SRV) image_usage_flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
        if (texture_desc.flags & GPUTextureFlag::UAV) image_usage_flags |= VK_IMAGE_USAGE_STORAGE_BIT;
        if (texture_desc.flags & GPUTextureFlag::TransferSrc) image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        if (texture_desc.flags & GPUTextureFlag::TransferDst) image_usage_flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

        VkImageType texture_type = VK_IMAGE_TYPE_2D;
        VkImageViewType view_type = VK_IMAGE_VIEW_TYPE_2D;
        VkImageCreateFlags texture_flags = 0;
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
        }

        VkImageCreateInfo image_info = {};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.format = format;
        image_info.extent = { texture_desc.width, texture_desc.height, texture_desc.depth };
        image_info.mipLevels = texture_desc.mip_level_count;
        image_info.arrayLayers = texture_desc.layer_count;
        image_info.samples = static_cast<VkSampleCountFlagBits>(texture_desc.sample_count);
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage = image_usage_flags;
        image_info.imageType = texture_type;
        image_info.flags = texture_flags;

        VmaAllocationCreateInfo allocation_info = {};
        allocation_info.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        VkResult result = vmaCreateImage(g_allocator, &image_info, &allocation_info, &texture->texture, &texture->allocation, nullptr);
        assert(result == VK_SUCCESS);

        const bool is_depth = (texture_desc.flags & GPUTextureFlag::DepthTarget) != 0;
        VkImageAspectFlags aspect = is_depth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

        VkImageViewCreateInfo view_info = {};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = texture->texture;
        view_info.viewType = view_type;
        view_info.format = format;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = texture_desc.layer_count;
        view_info.subresourceRange.baseMipLevel = 0;

        if (texture_desc.flags & GPUTextureFlag::RenderTarget)
        {
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            view_info.subresourceRange.levelCount = 1;
            RHI::vkCreateImageView(g_logical_device, &view_info, nullptr, &texture->rtv);
        }

        if (texture_desc.flags & GPUTextureFlag::DepthTarget)
        {
            view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            view_info.subresourceRange.levelCount = 1;
            RHI::vkCreateImageView(g_logical_device, &view_info, nullptr, &texture->dsv);
        }

        if (texture_desc.flags & GPUTextureFlag::SRV)
        {
            view_info.subresourceRange.aspectMask = aspect;
            view_info.subresourceRange.levelCount = texture_desc.mip_level_count;
            RHI::vkCreateImageView(g_logical_device, &view_info, nullptr, &texture->srv);
        }

        if (texture_desc.flags & GPUTextureFlag::UAV)
        {
            view_info.subresourceRange.aspectMask = aspect;
            view_info.subresourceRange.levelCount = 1;
            RHI::vkCreateImageView(g_logical_device, &view_info, nullptr, &texture->uav);
        }

        return texture;
    }
    
    GPUSampler *VulkanRHI::CreateSampler(const GPUSamplerDesc &sampler_desc)
    {
        return nullptr;
    }
    
    GPUShaderResources *VulkanRHI::CreateBindGroup(const GPUBindGroupDesc &bind_group_desc)
    {
        VulkanGPUShaderResources* bind_group = new VulkanGPUShaderResources();

        VkDescriptorBindingFlags binding_flags[MAX_BIND_GROUP_ENTRIES];
        std::fill(binding_flags, binding_flags + bind_group_desc.entry_count, VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);

        VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info = {};
        binding_flags_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        binding_flags_info.bindingCount = bind_group_desc.entry_count;
        binding_flags_info.pBindingFlags = binding_flags;

        VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[MAX_BIND_GROUP_ENTRIES];
        memset(descriptor_set_layout_bindings, 0, sizeof(VkDescriptorSetLayoutBinding) * MAX_BIND_GROUP_ENTRIES);

        for (uint32_t index = 0; index < bind_group_desc.entry_count; ++index)
        {
            descriptor_set_layout_bindings[index].binding = bind_group_desc.entries[index].position;
            descriptor_set_layout_bindings[index].descriptorCount = bind_group_desc.entries[index].count;
            descriptor_set_layout_bindings[index].stageFlags = VK_SHADER_STAGE_ALL;

            switch (bind_group_desc.entries[index].bind_type)
            {
                case GPUBindGroupEntryType::StorageBuffer: descriptor_set_layout_bindings[index].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; break;
                case GPUBindGroupEntryType::UniformBuffer: descriptor_set_layout_bindings[index].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; break;
                case GPUBindGroupEntryType::SampledTexture: descriptor_set_layout_bindings[index].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE; break;
                case GPUBindGroupEntryType::StorageTexture: descriptor_set_layout_bindings[index].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; break;
                case GPUBindGroupEntryType::Sampler: descriptor_set_layout_bindings[index].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER; break;
            }
        }

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};
        descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.pNext = &binding_flags_info;
        descriptor_set_layout_create_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        descriptor_set_layout_create_info.bindingCount = bind_group_desc.entry_count;
        descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings;

        VkResult result = RHI::vkCreateDescriptorSetLayout(g_logical_device, &descriptor_set_layout_create_info, nullptr, &bind_group->descriptor_set_layout);
        assert(result == VK_SUCCESS);

        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, g_adapter.max_storage_buffers },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, g_adapter.max_uniform_buffers },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, g_adapter.max_sampled_textures },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, g_adapter.max_storage_textures },
            { VK_DESCRIPTOR_TYPE_SAMPLER, g_adapter.max_samplers },
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        result = RHI::vkCreateDescriptorPool(g_logical_device, &pool_info, nullptr, &bind_group->descriptor_pool);
        assert(result == VK_SUCCESS);

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = bind_group->descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &bind_group->descriptor_set_layout;

        result = RHI::vkAllocateDescriptorSets(g_logical_device, &alloc_info, &bind_group->descriptor_set);
        assert(result == VK_SUCCESS);

        return bind_group;
    }
    
    GPUPipeline *VulkanRHI::CreateGraphicsPipeline(const GPUGraphicsPipelineDesc &pipeline_desc)
    {
        VulkanGPUGraphicsPipeline* graphics_pipeline = (VulkanGPUGraphicsPipeline*)malloc(sizeof(VulkanGPUGraphicsPipeline));
        
        const VulkanGPUShader* vertex_shader = static_cast<const VulkanGPUShader*>(pipeline_desc.vertex_shader);
        const VulkanGPUShader* fragment_shader = static_cast<const VulkanGPUShader*>(pipeline_desc.fragment_shader);

        VkPipelineShaderStageCreateInfo shader_stages[2] = {};
        shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[0].module = vertex_shader->module;
        shader_stages[0].pName = "main";
        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = fragment_shader->module;
        shader_stages[1].pName = "main";

        // No vertex input - route through vertex pulling
        VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {};
        vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = GetNativeTopology(pipeline_desc.topology);

        VkDynamicState dynamic_states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = std::size(dynamic_states);
        dynamic_state_create_info.pDynamicStates = dynamic_states;

        VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
        viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.scissorCount = 1;

        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
        rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.cullMode = GetNativeCullMode(pipeline_desc.cull_mode);
        rasterization_state_create_info.frontFace = GetNativeFrontFace(pipeline_desc.front_face);
        rasterization_state_create_info.polygonMode = GetNativePolygonMode(pipeline_desc.polygon_mode);
        rasterization_state_create_info.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = {};
        depth_stencil_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil_state_create_info.depthTestEnable = pipeline_desc.depth_test_enable;
        depth_stencil_state_create_info.depthWriteEnable = pipeline_desc.depth_write_enable;
        depth_stencil_state_create_info.depthCompareOp = GetNativeCompareOp(pipeline_desc.depth_compare_op);

        VkPipelineColorBlendAttachmentState blend_attachment_states[8] = {};
        for (uint32_t i = 0; i < pipeline_desc.color_format_count; i++)
        {
            blend_attachment_states[i].blendEnable = pipeline_desc.blend_enable;
            blend_attachment_states[i].srcColorBlendFactor = GetNativeBlendFactor(pipeline_desc.src_blend_factor_color);
            blend_attachment_states[i].dstColorBlendFactor = GetNativeBlendFactor(pipeline_desc.dst_blend_factor_color);
            blend_attachment_states[i].colorBlendOp = GetNativeBlendOp(pipeline_desc.blend_op_color);
            blend_attachment_states[i].srcAlphaBlendFactor = GetNativeBlendFactor(pipeline_desc.src_blend_factor_alpha);
            blend_attachment_states[i].dstAlphaBlendFactor = GetNativeBlendFactor(pipeline_desc.dst_blend_factor_alpha);
            blend_attachment_states[i].alphaBlendOp = GetNativeBlendOp(pipeline_desc.blend_op_alpha);
            blend_attachment_states[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        }

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
        color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.attachmentCount = pipeline_desc.color_format_count;
        color_blend_state_create_info.pAttachments = blend_attachment_states;

        VkFormat color_formats[8];
        for (uint32_t index = 0; index < pipeline_desc.color_format_count; index++)
        {
            color_formats[index] = GetNativeFormat(pipeline_desc.color_formats[index]);
        }

        VkPipelineRenderingCreateInfo rendering_create_info = {};
        rendering_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        rendering_create_info.colorAttachmentCount = pipeline_desc.color_format_count;
        rendering_create_info.pColorAttachmentFormats = color_formats;
        if (pipeline_desc.depth_format != GPUFormat::Unknown)
        {
            rendering_create_info.depthAttachmentFormat = GetNativeFormat(pipeline_desc.depth_format);
        }
            
        VkDescriptorSetLayout descriptor_set_layouts[4];
        for (uint32_t index = 0; index < pipeline_desc.bind_group_count; index++)
        {
            descriptor_set_layouts[index] = static_cast<const VulkanGPUShaderResources*>(pipeline_desc.bind_group_data[index])->descriptor_set_layout;
        }

        VkPushConstantRange push_constant_range = {};
        push_constant_range.stageFlags = GetNativeShaderStageFlags(pipeline_desc.push_constant_stage_flags);
        push_constant_range.size = max(pipeline_desc.push_constant_size, 128U);

        VkPipelineLayoutCreateInfo layout_info = {};
        layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_info.pSetLayouts = descriptor_set_layouts;
        layout_info.setLayoutCount = pipeline_desc.bind_group_count;
        layout_info.pushConstantRangeCount = 1;
        layout_info.pPushConstantRanges = &push_constant_range;

        VkResult result = RHI::vkCreatePipelineLayout(g_logical_device, &layout_info, nullptr, &graphics_pipeline->layout);
        assert(result == VK_SUCCESS);

        VkGraphicsPipelineCreateInfo pipeline_info = {};
        pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_info.pNext = &rendering_create_info;
        pipeline_info.stageCount = 2;
        pipeline_info.pStages = shader_stages;
        pipeline_info.pVertexInputState = &vertex_input_create_info;
        pipeline_info.pInputAssemblyState = &input_assembly_create_info;
        pipeline_info.pViewportState = &viewport_state_create_info;
        pipeline_info.pRasterizationState = &rasterization_state_create_info;
        pipeline_info.pMultisampleState = &multisample_state_create_info;
        pipeline_info.pDepthStencilState = &depth_stencil_state_create_info;
        pipeline_info.pColorBlendState = &color_blend_state_create_info;
        pipeline_info.pDynamicState = &dynamic_state_create_info;
        pipeline_info.layout = graphics_pipeline->layout;

        result = RHI::vkCreateGraphicsPipelines(g_logical_device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline->pipeline);
        assert(result == VK_SUCCESS);

        return graphics_pipeline;
    }
    
    GPUShader *VulkanRHI::CreateShader(const GPUShaderDesc &shader_desc)
    {
        VulkanGPUShader* native_shader = new VulkanGPUShader();

        VkShaderModuleCreateInfo module_info = {};
        module_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        module_info.codeSize = shader_desc.bytecode_size;
        module_info.pCode = static_cast<const uint32_t*>(shader_desc.bytecode);

        VkResult result = RHI::vkCreateShaderModule(g_logical_device, &module_info, nullptr, &native_shader->module);
        assert(result == VK_SUCCESS);

        return native_shader;
    }
    
    GPUSwapchain *VulkanRHI::CreateSwapchain(const GPUSwapchainDesc &desc)
    {
        return nullptr;
    }
    
    GPUCommandList *VulkanRHI::CreateCommandList()
    {
        return nullptr;
    }
}