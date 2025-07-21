#pragma once

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
    X(vkCmdCopyBuffer) \
    X(vkCmdPipelineBarrier) \
    X(vkCmdBeginRendering) \
    X(vkCmdEndRendering) \

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

#define VULKAN_ALL_FUNCTIONS(X) \
    VULKAN_GLOBAL_FUNCTIONS(X) \
    VULKAN_INSTANCE_FUNCTIONS(X) \
    VULKAN_DEVICE_FUNCTIONS(X)

namespace VulkanRHI
{
    // Opaque handle to dynamic library
    extern void* Library;

    #define DECLARE_PFN(name) extern PFN_##name name;
    VULKAN_ALL_FUNCTIONS(DECLARE_PFN)
    #undef DECLARE_PFN
}

void LoadVulkanLibrary();
void LoadVulkanGlobalFunc();
void LoadVulkanInstanceFunc(VkInstance Instance);
void LoadVulkanDeviceFunc(VkDevice Device);
VmaVulkanFunctions CreateVmaFuncTable();
void FreeVulkanLibrary();
