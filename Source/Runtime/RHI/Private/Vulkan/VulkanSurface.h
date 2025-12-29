#pragma once

#include <memory>

#include "Common/RHISurface.h"
#include "vulkan/vulkan_core.h"

class IRHICommandListContext;
class FVulkanDevice;
class VulkanSwapchain;

struct RHITexture;

class VulkanSurface : public RHISurface
{
public:
    VulkanSurface(FVulkanDevice& Device, RHISurfaceDesc& InDesc);
    ~VulkanSurface();

    RHITexture* get_back_buffer_texture() override;

    void acquire(IRHICommandListContext& ctx) override;
    void present(IRHICommandListContext& ctx) override;

    inline VkSurfaceKHR GetVkSurface() const;
    inline VkPresentModeKHR GetPresentMode() const;
    inline VkSurfaceFormatKHR GetSurfaceFormat() const;

    virtual std::shared_ptr<RHISemaphore> GetImageAvailableSemaphore() const override;
    virtual std::shared_ptr<RHISemaphore> GetRenderFinishedSemaphore() const override;

private:
    FVulkanDevice& Device;
    VulkanSwapchain* Swapchain;

    VkSurfaceKHR Surface;
    VkPresentModeKHR PresentMode;
    VkSurfaceFormatKHR SurfaceFormat;
};

inline VkSurfaceKHR VulkanSurface::GetVkSurface() const
{
    return Surface;
}

inline VkPresentModeKHR VulkanSurface::GetPresentMode() const
{
    return PresentMode;
}

inline VkSurfaceFormatKHR VulkanSurface::GetSurfaceFormat() const
{
    return SurfaceFormat;
}