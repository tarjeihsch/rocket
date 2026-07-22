#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0

#include "Vulkan/VulkanLoader.h"

#include <cassert>

#if defined(_WIN32)
    #include <windows.h>
#endif

#if defined(__linux__)
    #include <dlfcn.h>
#endif

namespace RHI
{
    static void* g_dynamic_library = nullptr;

    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
    PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = nullptr;
    PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = nullptr;
    PFN_vkCreateInstance vkCreateInstance = nullptr;
    PFN_vkDestroyInstance vkDestroyInstance = nullptr;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
    PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 = nullptr;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
    PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2 = nullptr;
    PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties = nullptr;
    PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties = nullptr;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties = nullptr;
    PFN_vkCreateDevice vkCreateDevice = nullptr;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDevice vkDestroyDevice = nullptr;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle = nullptr;
    PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;
    PFN_vkQueueSubmit2 vkQueueSubmit2 = nullptr;
    PFN_vkQueueWaitIdle vkQueueWaitIdle = nullptr;
    PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;
    PFN_vkAllocateMemory vkAllocateMemory = nullptr;
    PFN_vkFreeMemory vkFreeMemory = nullptr;
    PFN_vkMapMemory vkMapMemory = nullptr;
    PFN_vkUnmapMemory vkUnmapMemory = nullptr;
    PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges = nullptr;
    PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges = nullptr;
    PFN_vkBindBufferMemory vkBindBufferMemory = nullptr;
    PFN_vkBindBufferMemory2 vkBindBufferMemory2 = nullptr;
    PFN_vkBindImageMemory vkBindImageMemory = nullptr;
    PFN_vkBindImageMemory2 vkBindImageMemory2 = nullptr;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = nullptr;
    PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2 = nullptr;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
    PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2 = nullptr;
    PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements = nullptr;
    PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements = nullptr;
    PFN_vkCreateBuffer vkCreateBuffer = nullptr;
    PFN_vkDestroyBuffer vkDestroyBuffer = nullptr;
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress = nullptr;
    PFN_vkCreateImage vkCreateImage = nullptr;
    PFN_vkDestroyImage vkDestroyImage = nullptr;
    PFN_vkCreateImageView vkCreateImageView = nullptr;
    PFN_vkDestroyImageView vkDestroyImageView = nullptr;
    PFN_vkCreateSampler vkCreateSampler = nullptr;
    PFN_vkDestroySampler vkDestroySampler = nullptr;
    PFN_vkCreateShaderModule vkCreateShaderModule = nullptr;
    PFN_vkDestroyShaderModule vkDestroyShaderModule = nullptr;
    PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = nullptr;
    PFN_vkCreateComputePipelines vkCreateComputePipelines = nullptr;
    PFN_vkDestroyPipeline vkDestroyPipeline = nullptr;
    PFN_vkCreatePipelineLayout vkCreatePipelineLayout = nullptr;
    PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = nullptr;
    PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = nullptr;
    PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = nullptr;
    PFN_vkCreateDescriptorPool vkCreateDescriptorPool = nullptr;
    PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = nullptr;
    PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = nullptr;
    PFN_vkFreeDescriptorSets vkFreeDescriptorSets = nullptr;
    PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = nullptr;
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
    PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
    PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
    PFN_vkResetCommandPool vkResetCommandPool = nullptr;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer = nullptr;
    PFN_vkEndCommandBuffer vkEndCommandBuffer = nullptr;
    PFN_vkCmdBeginRendering vkCmdBeginRendering = nullptr;
    PFN_vkCmdEndRendering vkCmdEndRendering = nullptr;
    PFN_vkCmdBindPipeline vkCmdBindPipeline = nullptr;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = nullptr;
    PFN_vkCmdSetViewport vkCmdSetViewport = nullptr;
    PFN_vkCmdSetScissor vkCmdSetScissor = nullptr;
    PFN_vkCmdPushConstants vkCmdPushConstants = nullptr;
    PFN_vkCmdDraw vkCmdDraw = nullptr;
    PFN_vkCmdDrawIndexed vkCmdDrawIndexed = nullptr;
    PFN_vkCmdDrawIndirect vkCmdDrawIndirect = nullptr;
    PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect = nullptr;
    PFN_vkCmdDispatch vkCmdDispatch = nullptr;
    PFN_vkCmdCopyBuffer vkCmdCopyBuffer = nullptr;
    PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage = nullptr;
    PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer = nullptr;
    PFN_vkCmdCopyImage vkCmdCopyImage = nullptr;
    PFN_vkCmdCopyBuffer2 vkCmdCopyBuffer2 = nullptr;
    PFN_vkCmdBlitImage2 vkCmdBlitImage2 = nullptr;
    PFN_vkCmdCopyBufferToImage2 vkCmdCopyBufferToImage2 = nullptr;
    PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2 = nullptr;
    PFN_vkCreateSemaphore vkCreateSemaphore = nullptr;
    PFN_vkDestroySemaphore vkDestroySemaphore = nullptr;
    PFN_vkCreateFence vkCreateFence = nullptr;
    PFN_vkDestroyFence vkDestroyFence = nullptr;
    PFN_vkWaitForFences vkWaitForFences = nullptr;
    PFN_vkResetFences vkResetFences = nullptr;
    PFN_vkCreateQueryPool vkCreateQueryPool = nullptr;
    PFN_vkResetQueryPool vkResetQueryPool = nullptr;
    PFN_vkDestroyQueryPool vkDestroyQueryPool = nullptr;
    PFN_vkCmdResetQueryPool vkCmdResetQueryPool = nullptr;
    PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp = nullptr;
    PFN_vkCmdBeginQuery vkCmdBeginQuery = nullptr;
    PFN_vkCmdEndQuery vkCmdEndQuery = nullptr;
    PFN_vkGetQueryPoolResults vkGetQueryPoolResults = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = nullptr;
    PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = nullptr;
    PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = nullptr;

#define LOAD_GLOBAL(fn) \
    fn = reinterpret_cast<PFN_##fn>(vkGetInstanceProcAddr(nullptr, #fn))

#define LOAD_INSTANCE(instance, fn) \
    fn = reinterpret_cast<PFN_##fn>(vkGetInstanceProcAddr(instance, #fn))

#define LOAD_DEVICE(device, fn) \
    fn = reinterpret_cast<PFN_##fn>(vkGetDeviceProcAddr(device, #fn))

    void LoadVulkanLibrary()
    {
#if defined(_WIN32)
        g_dynamic_library = ::LoadLibraryA("vulkan-1.dll");
        vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(::GetProcAddress(static_cast<HMODULE>(g_dynamic_library), "vkGetInstanceProcAddr"));
#endif

#if defined(__linux__)
        g_dynamic_library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
        vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(g_dynamic_library, "vkGetInstanceProcAddr"));
#endif
    }

    void UnloadVulkanLibrary()
    {
#if defined(_WIN32)
        ::FreeLibrary(static_cast<HMODULE>(g_dynamic_library));
#endif

#if defined(__linux__)
        dlclose(g_dynamic_library);
#endif
    }

    void LoadVulkanGlobals()
    {
        LOAD_GLOBAL(vkEnumerateInstanceExtensionProperties);
        LOAD_GLOBAL(vkEnumerateInstanceLayerProperties);
        LOAD_GLOBAL(vkCreateInstance);
    }

    void LoadVulkanInstance(VkInstance instance)
    {
        LOAD_INSTANCE(instance, vkDestroyInstance);
        LOAD_INSTANCE(instance, vkEnumeratePhysicalDevices);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceProperties);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceProperties2);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceFeatures2);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceQueueFamilyProperties);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceMemoryProperties);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceMemoryProperties2);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceFormatProperties);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceImageFormatProperties);
        LOAD_INSTANCE(instance, vkEnumerateDeviceExtensionProperties);
        LOAD_INSTANCE(instance, vkCreateDevice);
        LOAD_INSTANCE(instance, vkGetDeviceProcAddr);
        LOAD_INSTANCE(instance, vkDestroySurfaceKHR);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceSurfaceSupportKHR);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceSurfaceFormatsKHR);
        LOAD_INSTANCE(instance, vkGetPhysicalDeviceSurfacePresentModesKHR);
        LOAD_INSTANCE(instance, vkCreateDebugUtilsMessengerEXT);
        LOAD_INSTANCE(instance, vkDestroyDebugUtilsMessengerEXT);
    }

    void LoadVulkanDevice(VkDevice device)
    {
        LOAD_DEVICE(device, vkDestroyDevice);
        LOAD_DEVICE(device, vkDeviceWaitIdle);
        LOAD_DEVICE(device, vkGetDeviceQueue);
        LOAD_DEVICE(device, vkQueueSubmit2);
        LOAD_DEVICE(device, vkQueueWaitIdle);
        LOAD_DEVICE(device, vkQueuePresentKHR);
        LOAD_DEVICE(device, vkAllocateMemory);
        LOAD_DEVICE(device, vkFreeMemory);
        LOAD_DEVICE(device, vkMapMemory);
        LOAD_DEVICE(device, vkUnmapMemory);
        LOAD_DEVICE(device, vkFlushMappedMemoryRanges);
        LOAD_DEVICE(device, vkInvalidateMappedMemoryRanges);
        LOAD_DEVICE(device, vkBindBufferMemory);
        LOAD_DEVICE(device, vkBindBufferMemory2);
        LOAD_DEVICE(device, vkBindImageMemory);
        LOAD_DEVICE(device, vkBindImageMemory2);
        LOAD_DEVICE(device, vkGetBufferMemoryRequirements);
        LOAD_DEVICE(device, vkGetBufferMemoryRequirements2);
        LOAD_DEVICE(device, vkGetImageMemoryRequirements);
        LOAD_DEVICE(device, vkGetImageMemoryRequirements2);
        LOAD_DEVICE(device, vkGetDeviceBufferMemoryRequirements);
        LOAD_DEVICE(device, vkGetDeviceImageMemoryRequirements);
        LOAD_DEVICE(device, vkCreateBuffer);
        LOAD_DEVICE(device, vkDestroyBuffer);
        LOAD_DEVICE(device, vkGetBufferDeviceAddress);
        LOAD_DEVICE(device, vkCreateImage);
        LOAD_DEVICE(device, vkDestroyImage);
        LOAD_DEVICE(device, vkCreateImageView);
        LOAD_DEVICE(device, vkDestroyImageView);
        LOAD_DEVICE(device, vkCreateSampler);
        LOAD_DEVICE(device, vkDestroySampler);
        LOAD_DEVICE(device, vkCreateShaderModule);
        LOAD_DEVICE(device, vkDestroyShaderModule);
        LOAD_DEVICE(device, vkCreateGraphicsPipelines);
        LOAD_DEVICE(device, vkCreateComputePipelines);
        LOAD_DEVICE(device, vkDestroyPipeline);
        LOAD_DEVICE(device, vkCreatePipelineLayout);
        LOAD_DEVICE(device, vkDestroyPipelineLayout);
        LOAD_DEVICE(device, vkCreateDescriptorSetLayout);
        LOAD_DEVICE(device, vkDestroyDescriptorSetLayout);
        LOAD_DEVICE(device, vkCreateDescriptorPool);
        LOAD_DEVICE(device, vkDestroyDescriptorPool);
        LOAD_DEVICE(device, vkAllocateDescriptorSets);
        LOAD_DEVICE(device, vkFreeDescriptorSets);
        LOAD_DEVICE(device, vkUpdateDescriptorSets);
        LOAD_DEVICE(device, vkCreateSwapchainKHR);
        LOAD_DEVICE(device, vkDestroySwapchainKHR);
        LOAD_DEVICE(device, vkGetSwapchainImagesKHR);
        LOAD_DEVICE(device, vkAcquireNextImageKHR);
        LOAD_DEVICE(device, vkCreateCommandPool);
        LOAD_DEVICE(device, vkDestroyCommandPool);
        LOAD_DEVICE(device, vkResetCommandPool);
        LOAD_DEVICE(device, vkAllocateCommandBuffers);
        LOAD_DEVICE(device, vkBeginCommandBuffer);
        LOAD_DEVICE(device, vkEndCommandBuffer);
        LOAD_DEVICE(device, vkCmdBeginRendering);
        LOAD_DEVICE(device, vkCmdEndRendering);
        LOAD_DEVICE(device, vkCmdBindPipeline);
        LOAD_DEVICE(device, vkCmdBindDescriptorSets);
        LOAD_DEVICE(device, vkCmdSetViewport);
        LOAD_DEVICE(device, vkCmdSetScissor);
        LOAD_DEVICE(device, vkCmdPushConstants);
        LOAD_DEVICE(device, vkCmdDraw);
        LOAD_DEVICE(device, vkCmdDrawIndexed);
        LOAD_DEVICE(device, vkCmdDrawIndirect);
        LOAD_DEVICE(device, vkCmdDrawIndexedIndirect);
        LOAD_DEVICE(device, vkCmdDispatch);
        LOAD_DEVICE(device, vkCmdCopyBuffer);
        LOAD_DEVICE(device, vkCmdCopyBufferToImage);
        LOAD_DEVICE(device, vkCmdCopyImageToBuffer);
        LOAD_DEVICE(device, vkCmdCopyImage);
        LOAD_DEVICE(device, vkCmdCopyBuffer2);
        LOAD_DEVICE(device, vkCmdBlitImage2);
        LOAD_DEVICE(device, vkCmdPipelineBarrier2);
        LOAD_DEVICE(device, vkCmdCopyBufferToImage2);
        LOAD_DEVICE(device, vkCreateSemaphore);
        LOAD_DEVICE(device, vkDestroySemaphore);
        LOAD_DEVICE(device, vkCreateFence);
        LOAD_DEVICE(device, vkDestroyFence);
        LOAD_DEVICE(device, vkWaitForFences);
        LOAD_DEVICE(device, vkResetFences);
        LOAD_DEVICE(device, vkCreateQueryPool);
        LOAD_DEVICE(device, vkResetQueryPool);
        LOAD_DEVICE(device, vkDestroyQueryPool);
        LOAD_DEVICE(device, vkCmdResetQueryPool);
        LOAD_DEVICE(device, vkCmdWriteTimestamp);
        LOAD_DEVICE(device, vkGetQueryPoolResults);
        LOAD_DEVICE(device, vkCmdBeginQuery);
        LOAD_DEVICE(device, vkCmdEndQuery);
        LOAD_DEVICE(device, vkSetDebugUtilsObjectNameEXT);
        LOAD_DEVICE(device, vkCmdBeginDebugUtilsLabelEXT);
        LOAD_DEVICE(device, vkCmdEndDebugUtilsLabelEXT);
    }
};
