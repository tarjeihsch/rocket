#pragma once

#include "Common/RHISwapchain.h"
#include "VulkanResourceManager.h"

#include <vector>
#include <vulkan/vulkan_core.h>
#include <memory>

class VulkanCommandListContext;
struct VulkanSemaphore;
class VulkanDevice;
struct VulkanTexture;
struct VulkanTextureView;

class VulkanSwapchain : public RHISwapchain
{
private:
    VulkanDevice& Device;

public:
    explicit VulkanSwapchain(VulkanDevice& InDevice);
    virtual ~VulkanSwapchain() override;

    RHITexture* GetBackBufferTexture() const;

    inline VkSwapchainKHR GetVkSwapchain() const;
    inline VkExtent2D GetFramebufferExtent() const;
    inline std::shared_ptr<VulkanSemaphore> get_image_acquired_semaphore() const;
    inline std::shared_ptr<VulkanSemaphore> get_render_finished_semaphore() const;
    
private:
    void CreateSwapchain(VkSurfaceKHR Surface, VkSurfaceFormatKHR SurfaceFormat, VkExtent2D Extent);
    void DestroySwapchain();

    bool AcquireBackBufferImage();
    void PresentBackBufferImage();

private:
    VkSwapchainKHR Swapchain;
    VkExtent2D Extent;

    std::vector<std::unique_ptr<VulkanTexture>> BackBuffers;
    std::vector<std::shared_ptr<VulkanSemaphore>> ImageAcquiredSemaphores;
    std::vector<std::shared_ptr<VulkanSemaphore>> RenderFinishedSemaphores;

    uint32_t CurrentImageIndex;
    uint32_t SemaphoreIndex;
    uint32_t NumAcquireCalls;
    uint32_t NumPresentCalls;

    friend class VulkanSurface;
};

inline VkSwapchainKHR VulkanSwapchain::GetVkSwapchain() const
{
    return Swapchain;
}

inline VkExtent2D VulkanSwapchain::GetFramebufferExtent() const
{
    return Extent;
}

inline std::shared_ptr<VulkanSemaphore> VulkanSwapchain::get_image_acquired_semaphore() const
{
    return ImageAcquiredSemaphores[SemaphoreIndex];
}

inline std::shared_ptr<VulkanSemaphore> VulkanSwapchain::get_render_finished_semaphore() const
{
    return RenderFinishedSemaphores[CurrentImageIndex];
}