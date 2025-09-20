#pragma once

#include "VulkanResourceManager.h"
#include "VulkanTexture.h"

#include <vulkan/vulkan_core.h>
#include <vector>

class FVulkanDevice;

class FVulkanViewport
{
private:
    FVulkanDevice& Device;

public:
    FVulkanViewport(FVulkanDevice& InDevice);
    ~FVulkanViewport();

    bool AcquireNextImageIndex();
    void Present();

    inline VkSurfaceKHR GetVkSurface() const;
    inline VkSwapchainKHR GetVkSwapchain() const;
    inline VkExtent2D GetExtent() const;
    inline VkPresentModeKHR GetPresentMode() const;
    inline VkSurfaceFormatKHR GetSurfaceFormat() const;

    inline VkSemaphore GetImageAcquiredSemaphore() const;
    inline VkSemaphore GetRenderFinishedSemaphore() const;
    inline VkImage GetImage() const;
    inline VkImageView GetImageView() const;

    inline FVulkanTexture* GetBackBufferTexture() const;
    inline FVulkanTextureView* GetBackBufferTextureView() const;

private:
    void CreateSwapchain();
    void DestroySwapchain();
    
private:
    VkSurfaceKHR Surface;
    VkSwapchainKHR Swapchain;
    VkExtent2D Extent;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;

    std::vector<FVulkanTexture*> BackBuffer;
    std::vector<FVulkanTextureView*> BackBufferView;

    std::vector<VkSemaphore> ImageAcquiredSemaphores;
    std::vector<VkSemaphore> RenderFinishedSemaphores;

    uint32_t CurrentImageIndex;
    uint32_t SemaphoreIndex;
    uint32_t NumAcquireCalls;
    uint32_t NumPresentCalls;
};

inline VkSurfaceKHR FVulkanViewport::GetVkSurface() const
{
    return Surface;
}

inline VkSwapchainKHR FVulkanViewport::GetVkSwapchain() const
{
    return Swapchain;
}

inline VkExtent2D FVulkanViewport::GetExtent() const
{
    return Extent;
}

inline VkPresentModeKHR FVulkanViewport::GetPresentMode() const
{
    return PresentMode;
}

inline VkSurfaceFormatKHR FVulkanViewport::GetSurfaceFormat() const
{
    return SurfaceFormat;
}

inline VkSemaphore FVulkanViewport::GetImageAcquiredSemaphore() const
{
    return ImageAcquiredSemaphores[SemaphoreIndex];
}

inline VkSemaphore FVulkanViewport::GetRenderFinishedSemaphore() const
{
    return RenderFinishedSemaphores[CurrentImageIndex];
}

inline VkImage FVulkanViewport::GetImage() const
{
    return BackBuffer[CurrentImageIndex]->Image;
}

inline VkImageView FVulkanViewport::GetImageView() const
{
    return BackBufferView[CurrentImageIndex]->ImageView;
}

inline FVulkanTexture * FVulkanViewport::GetBackBufferTexture() const
{
    return BackBuffer[CurrentImageIndex];
}

inline FVulkanTextureView * FVulkanViewport::GetBackBufferTextureView() const
{
    return BackBufferView[CurrentImageIndex];
}