#pragma once

enum class ERHIQueueSyncStage : uint8_t;
enum class ERHITextureLayout : uint8_t;
enum class ERHIBorderColor : uint8_t;
enum class ERHITextureUsage : uint8_t;
enum class ERHITextureFormat;
enum class ERHITextureAspect;
enum class ERHIFilterMode : uint8_t;
enum class ERHIFormat : uint8_t;
enum class ERHIAddressMode : uint8_t;
enum class ERHICompareOp : uint8_t;
enum class ERHIBlendFactor : uint8_t;
enum class ERHIBlendOp : uint8_t;
enum class ERHIPrimitiveTopology : uint8_t;
enum class ERHICullMode : uint8_t;
enum class ERHIPolygonMode : uint8_t;
enum class ERHIResourceAccess : uint16_t;
enum class ERHIPipelineStage : uint16_t;

#if defined(_WIN32)
    #define VK_USE_PLATFORM_WIN32_KHR
    #include <Windows.h>
    #undef APIENTRY
#endif

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VULKAN_GLOBAL_FUNCTIONS(X) \
    X(vkGetInstanceProcAddr) \
    X(vkEnumerateInstanceExtensionProperties) \
    X(vkEnumerateInstanceLayerProperties) \
    X(vkCreateInstance)

#define VULKAN_INSTANCE_FUNCTIONS(X) \
    X(vkDestroyInstance) \
    X(vkEnumeratePhysicalDevices) \
    X(vkGetPhysicalDeviceProperties) \
    X(vkGetPhysicalDeviceFeatures) \
    X(vkGetPhysicalDeviceQueueFamilyProperties) \
    X(vkGetPhysicalDeviceMemoryProperties) \
    X(vkGetPhysicalDeviceMemoryProperties2) \
    X(vkGetPhysicalDeviceMemoryProperties2KHR) \
    X(vkEnumerateDeviceExtensionProperties) \
    X(vkGetPhysicalDeviceFormatProperties) \
    X(vkGetPhysicalDeviceFormatProperties2) \
    X(vkGetPhysicalDeviceFormatProperties2KHR) \
    X(vkGetPhysicalDeviceImageFormatProperties) \
    X(vkGetPhysicalDeviceProperties2) \
    X(vkGetPhysicalDeviceProperties2KHR) \
    X(vkGetPhysicalDeviceFeatures2) \
    X(vkGetPhysicalDeviceFeatures2KHR) \
    X(vkGetPhysicalDeviceSurfaceSupportKHR) \
    X(vkGetPhysicalDeviceSurfaceCapabilitiesKHR) \
    X(vkGetPhysicalDeviceSurfaceFormatsKHR) \
    X(vkGetPhysicalDeviceSurfacePresentModesKHR) \
    X(vkCreateDebugUtilsMessengerEXT) \
    X(vkDestroyDebugUtilsMessengerEXT) \
    X(vkCreateDevice) \
    X(vkGetDeviceProcAddr) \
    X(vkDestroySurfaceKHR)

#define VULKAN_DEVICE_FUNCTIONS(X) \
    X(vkDestroyDevice) \
    X(vkDeviceWaitIdle) \
    X(vkGetDeviceQueue) \
    X(vkAllocateMemory) \
    X(vkFreeMemory) \
    X(vkMapMemory) \
    X(vkUnmapMemory) \
    X(vkFlushMappedMemoryRanges) \
    X(vkInvalidateMappedMemoryRanges) \
    X(vkGetBufferMemoryRequirements) \
    X(vkGetBufferMemoryRequirements2) \
    X(vkGetBufferMemoryRequirements2KHR) \
    X(vkGetImageMemoryRequirements) \
    X(vkGetImageMemoryRequirements2) \
    X(vkGetImageMemoryRequirements2KHR) \
    X(vkBindBufferMemory) \
    X(vkBindBufferMemory2) \
    X(vkBindBufferMemory2KHR) \
    X(vkBindImageMemory) \
    X(vkBindImageMemory2) \
    X(vkBindImageMemory2KHR) \
    X(vkCreateBuffer) \
    X(vkDestroyBuffer) \
    X(vkCreateImage) \
    X(vkDestroyImage) \
    X(vkGetImageSubresourceLayout) \
    X(vkCreateImageView) \
    X(vkDestroyImageView) \
    X(vkCreateCommandPool) \
    X(vkDestroyCommandPool) \
    X(vkAllocateCommandBuffers) \
    X(vkFreeCommandBuffers) \
    X(vkBeginCommandBuffer) \
    X(vkEndCommandBuffer) \
    X(vkResetCommandBuffer) \
    X(vkQueueSubmit) \
    X(vkQueueSubmit2) \
    X(vkQueueWaitIdle) \
    X(vkQueuePresentKHR) \
    X(vkCreateSemaphore) \
    X(vkDestroySemaphore) \
    X(vkWaitSemaphores) \
    X(vkCreateFence) \
    X(vkDestroyFence) \
    X(vkWaitForFences) \
    X(vkResetFences) \
    X(vkCreateSwapchainKHR) \
    X(vkDestroySwapchainKHR) \
    X(vkGetSwapchainImagesKHR) \
    X(vkAcquireNextImageKHR) \
    X(vkGetDeviceBufferMemoryRequirements) \
    X(vkGetDeviceImageMemoryRequirements) \
    X(vkCreateShaderModule) \
    X(vkDestroyShaderModule) \
    X(vkCreateGraphicsPipelines) \
    X(vkDestroyPipeline) \
    X(vkCmdBindPipeline) \
    X(vkCmdBindDescriptorSets) \
    X(vkCmdBindVertexBuffers) \
    X(vkCreateDescriptorSetLayout) \
    X(vkCreateDescriptorPool) \
    X(vkAllocateDescriptorSets) \
    X(vkUpdateDescriptorSets) \
    X(vkCmdBindIndexBuffer) \
    X(vkCmdDraw) \
    X(vkCmdBlitImage) \
    X(vkCmdDrawIndexed) \
    X(vkDestroyDescriptorPool) \
    X(vkDestroyDescriptorSetLayout)\
    X(vkCreatePipelineLayout) \
    X(vkDestroyPipelineLayout) \
    X(vkCmdSetViewport) \
    X(vkCmdSetScissor) \
    X(vkCmdCopyBuffer) \
    X(vkCmdPipelineBarrier) \
    X(vkCmdPipelineBarrier2) \
    X(vkCmdBeginRendering) \
    X(vkCmdEndRendering) \
    X(vkCmdPushConstants) \
    X(vkCmdCopyBufferToImage) \
    X(vkCreateSampler) \
    X(vkDestroySampler)

#define VMA_BASE_FUNCTIONS(X) \
    X(vkGetInstanceProcAddr) \
    X(vkGetDeviceProcAddr) \
    X(vkGetPhysicalDeviceProperties) \
    X(vkGetPhysicalDeviceMemoryProperties) \
    X(vkAllocateMemory) \
    X(vkFreeMemory) \
    X(vkMapMemory) \
    X(vkUnmapMemory) \
    X(vkFlushMappedMemoryRanges) \
    X(vkInvalidateMappedMemoryRanges) \
    X(vkBindBufferMemory) \
    X(vkBindImageMemory) \
    X(vkGetBufferMemoryRequirements) \
    X(vkGetImageMemoryRequirements) \
    X(vkCreateBuffer) \
    X(vkDestroyBuffer) \
    X(vkCreateImage) \
    X(vkDestroyImage) \
    X(vkCmdCopyBuffer)

#define VULKAN_ALL_FUNCTIONS(X)                                                                                                                                                                                                                                                                                                                                                                                                                                                                \
    VULKAN_GLOBAL_FUNCTIONS(X)                                                                                                                                                                                                                                                                                                                                                                                                                                                                 \
    VULKAN_INSTANCE_FUNCTIONS(X)                                                                                                                                                                                                                                                                                                                                                                                                                                                               \
    VULKAN_DEVICE_FUNCTIONS(X)

enum class ERHIResourceMemory;
enum class ERHIBufferUsage : uint32_t;

namespace VulkanRHI
{
    // Opaque handle to Vulkan dynamic library
    extern void* Library;

#define DECLARE_PFN(name) extern PFN_##name name;
    VULKAN_ALL_FUNCTIONS(DECLARE_PFN)
#undef DECLARE_PFN

    VkImageCreateInfo GetVkImageCreateInfo(VkExtent3D Extent, VkFormat Format, VkImageUsageFlags UsageFlags);
    VkImageViewCreateInfo GetVkImageViewCreateInfo(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags);
    VkBufferCreateInfo GetVkBufferCreateInfo(uint64_t Size, VkBufferUsageFlags UsageFlags);

    VkImageLayout GetVulkanLayout(ERHITextureLayout layout);
    VkAccessFlags GetVulkanAccess(ERHIResourceAccess access);
    VkPipelineStageFlags GetVulkanPipelineStage(ERHIPipelineStage stage);
    VkImageUsageFlags GetVulkanUsage(ERHITextureUsage usage);
    VkFormat GetVkFormat(ERHITextureFormat Format);
    VkImageAspectFlags GetVkAspect(ERHITextureAspect Aspect);
    VkFormat ToVkFormat(ERHIFormat Format);
    VkPrimitiveTopology ToVkTopology(ERHIPrimitiveTopology Topology);
    VkCullModeFlags ToVkCullMode(ERHICullMode Mode);
    VkPolygonMode ToVkPolygonMode(ERHIPolygonMode Mode);
    VmaMemoryUsage ToVmaMemoryUsage(ERHIResourceMemory Memory);

    VkBufferUsageFlags GetVkBufferUsage(ERHIBufferUsage usage);

    VkFilter ToVkFilter(ERHIFilterMode mode);
    VkSamplerAddressMode ToVkAddressMode(ERHIAddressMode mode);
    VkBorderColor ToVkBorderColor(ERHIBorderColor color);
    VkCompareOp ToVkCompareOp(ERHICompareOp op);

    VkBlendFactor GetVkBlendFactor(ERHIBlendFactor Factor);
    VkBlendOp GetVkBlendOp(ERHIBlendOp Op);

    VkPipelineStageFlags2 ToVkStage2(ERHIQueueSyncStage Stage);
}

void LoadVulkanLibrary();
void LoadVulkanGlobalFunc();
void LoadVulkanInstanceFunc(VkInstance Instance);
void LoadVulkanDeviceFunc(VkDevice Device);
VmaVulkanFunctions CreateVmaFuncTable();
void FreeVulkanLibrary();