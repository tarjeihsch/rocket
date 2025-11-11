#pragma once

#include "Common/RHISwapchain.h"
#include "VulkanResourceManager.h"

#include <vector>
#include <vulkan/vulkan_core.h>
#include <memory>

class FVulkanDevice;
struct FVulkanTexture;
struct FVulkanTextureView;

class RVulkanSwapchain : public RRHISwapchain
{
private:
    FVulkanDevice& Device;

public:
    explicit RVulkanSwapchain(FVulkanDevice& InDevice);
    virtual ~RVulkanSwapchain() override;

    virtual bool AcquireNextImageIndex() override;
    virtual void Present() override;
    virtual FRHITexture* GetBackBufferTexture() const override;
    virtual FRHITextureView* GetBackBufferTextureView() const override;

    inline VkSurfaceKHR GetVkSurface() const;
    inline VkSwapchainKHR GetVkSwapchain() const;
    inline VkExtent2D GetFramebufferExtent() const;
    inline VkPresentModeKHR GetPresentMode() const;
    inline VkSurfaceFormatKHR GetSurfaceFormat() const;
    inline VkSemaphore GetImageAcquiredSemaphore() const;
    inline VkSemaphore GetRenderFinishedSemaphore() const;
    
    VkImage GetImage() const;
    VkImageView GetImageView() const;

private:
    void CreateSwapchain();
    void DestroySwapchain();
    
private:
    VkSurfaceKHR Surface;
    VkSwapchainKHR Swapchain;
    VkExtent2D Extent;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;

    std::vector<std::unique_ptr<FVulkanTexture>> Backbuffer;
    std::vector<std::unique_ptr<FVulkanTextureView>> BackbufferView;

    std::vector<VkSemaphore> ImageAcquiredSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;

    uint32_t CurrentImageIndex;
    uint32_t SemaphoreIndex;
    uint32_t NumAcquireCalls;
    uint32_t NumPresentCalls;
};

inline VkSurfaceKHR RVulkanSwapchain::GetVkSurface() const
{
    return Surface;
}

inline VkSwapchainKHR RVulkanSwapchain::GetVkSwapchain() const
{
    return Swapchain;
}

inline VkExtent2D RVulkanSwapchain::GetFramebufferExtent() const
{
    return Extent;
}

inline VkPresentModeKHR RVulkanSwapchain::GetPresentMode() const
{
    return PresentMode;
}

inline VkSurfaceFormatKHR RVulkanSwapchain::GetSurfaceFormat() const
{
    return SurfaceFormat;
}

inline VkSemaphore RVulkanSwapchain::GetImageAcquiredSemaphore() const
{
    return ImageAcquiredSemaphores[SemaphoreIndex];
}

inline VkSemaphore RVulkanSwapchain::GetRenderFinishedSemaphore() const
{
    return RenderFinishedSemaphores[CurrentImageIndex];
}