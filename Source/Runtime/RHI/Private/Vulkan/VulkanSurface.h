#pragma once

#include <memory>

#include "Common/RHISurface.h"
#include "vulkan/vulkan_core.h"

class VulkanCommandBufferContext;
class RHICommandListContext;
class VulkanDevice;
class VulkanSwapchain;

struct RHITexture;

class VulkanSurface : public RHISurface
{
public:
    VulkanSurface(VulkanDevice& Device, const RHISurfaceDesc& InDesc);
    ~VulkanSurface();

    void BeginFrame() override;
	void EndFrame() override;
    virtual RHITexture* GetBackBufferTexture() override;
    virtual std::shared_ptr<RHISemaphore> GetImageAvailableSemaphore() const override;
    virtual std::shared_ptr<RHISemaphore> GetRenderFinishedSemaphore() const override;

    inline VkSurfaceKHR GetVkSurface() const;
    inline VkPresentModeKHR GetPresentMode() const;
    inline VkSurfaceFormatKHR GetSurfaceFormat() const;

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