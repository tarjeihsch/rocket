#pragma once

#include <memory>

#include "Common/RHISurface.h"
#include "vulkan/vulkan_core.h"

class RHICommandListContext;
class VulkanDevice;
class VulkanSwapchain;

struct RHITexture;

class VulkanSurface : public RHISurface
{
public:
    VulkanSurface(VulkanDevice& Device, RHISurfaceDesc& InDesc);
    ~VulkanSurface();

    RHITexture* get_back_buffer_texture() override;

    void acquire(RHICommandListContext& ctx) override;
    void present(RHICommandListContext& ctx) override;

    inline VkSurfaceKHR GetVkSurface() const;
    inline VkPresentModeKHR GetPresentMode() const;
    inline VkSurfaceFormatKHR GetSurfaceFormat() const;

    virtual std::shared_ptr<RHISemaphore> GetImageAvailableSemaphore() const override;
    virtual std::shared_ptr<RHISemaphore> GetRenderFinishedSemaphore() const override;

private:
    VulkanDevice& Device;
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