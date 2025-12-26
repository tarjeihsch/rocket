#pragma once

#include <memory>

#include "Common/RHIViewport.h"
#include "vulkan/vulkan_core.h"

class FVulkanDevice;
class RVulkanSwapchain;

struct FRHITexture;

class RVulkanViewport : public RRHIViewport
{
public:
    RVulkanViewport(FVulkanDevice& Device, RRHIViewportDesc& InDesc);
    ~RVulkanViewport();

    FRHITexture* GetBackBufferTexture() override;
    Vector2i GetExtent() const override;

    void Acquire() override;
    void Present();

    inline VkSurfaceKHR GetVkSurface() const;
    inline VkPresentModeKHR GetPresentMode() const;
    inline VkSurfaceFormatKHR GetSurfaceFormat() const;

    virtual const std::shared_ptr<FRHISemaphore>& GetImageAvailableSemaphore() const override;
    virtual const std::shared_ptr<FRHISemaphore>& GetRenderFinishedSemaphore() const override;

private:
    FVulkanDevice& Device;
    RVulkanSwapchain* Swapchain;

    VkSurfaceKHR Surface;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;
};

inline VkSurfaceKHR RVulkanViewport::GetVkSurface() const
{
    return Surface;
}

inline VkPresentModeKHR RVulkanViewport::GetPresentMode() const
{
    return PresentMode;
}

inline VkSurfaceFormatKHR RVulkanViewport::GetSurfaceFormat() const
{
    return SurfaceFormat;
}