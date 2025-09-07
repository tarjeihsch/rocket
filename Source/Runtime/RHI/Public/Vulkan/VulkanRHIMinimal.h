#pragma once

#include "Common/RHI.h"
#include "vma/vk_mem_alloc.h"

#include <vulkan/vulkan_core.h>
#include <span>
#include <assert.h>

class IVulkanRHIMinimal : public IRHI
{
public:
    static constexpr ERHIInterfaceType StaticType = ERHIInterfaceType::Vulkan;

    virtual VkInstance RHIGetVkInstance() const = 0;
    virtual VkDevice RHIGetVkDevice() const = 0;
    virtual VkPhysicalDevice RHIGetVkPhysicalDevice() const = 0;
    virtual VmaAllocator RHIGetVmaAllocator() const = 0;
    virtual VkQueue RHIGetGraphicsVkQueue() const = 0;
    virtual VkQueue RHIGetComputeVkQueue() const = 0;
    virtual VkQueue RHIGetTransferVkQueue() const = 0;
    virtual VkQueue RHIGetPresentVkQueue() const = 0;
    virtual uint32_t RHIGetGraphicsQueueFamilyIndex() const = 0;
    virtual uint32_t RHIGetComputeQueueFamilyIndex() const = 0;
    virtual uint32_t RHIGetTransferQueueFamilyIndex() const = 0;
    virtual uint32_t RHIGetPresentQueueFamilyIndex() const = 0;
    virtual VkSurfaceKHR RHIGetVkSurface() const = 0;
    virtual VkSwapchainKHR RHIGetVkSwapchain() const = 0;
    virtual VkExtent2D RHIGetSwapchainExtent() const = 0;
    virtual VkPresentModeKHR RHIGetSwapchainPresentMode() const = 0;
    virtual VkSurfaceFormatKHR RHIGetSwapchainSurfaceFormat() const = 0;
    virtual VkSemaphore GetImageAcquiredSemaphore() const = 0;
    virtual VkSemaphore GetRenderFinishedSemaphore() const = 0;
    virtual VkPipelineLayout GetBindlessPipelineLayout() const = 0;
    virtual VkDescriptorSet GetBindlessDescriptorSet() const = 0;
    virtual VkDescriptorSetLayout GetBindlessDesciptorSetLayout() const = 0;
    virtual std::span<const char*> GetValidationExtensions() = 0;
    virtual std::span<const char*> GetInstanceExtensions() = 0;
    virtual std::span<const char *> GetPhysicalDeviceExtensions() = 0;
};

inline IVulkanRHIMinimal* GetVulkanRHIMinimal()
{
    assert(GRHI->GetInterfaceType() == IVulkanRHIMinimal::StaticType);
    return GetRHI<IVulkanRHIMinimal>();
}
