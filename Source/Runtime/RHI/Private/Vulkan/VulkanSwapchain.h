#pragma once

#include "Common/RHISwapchain.h"
#include "VulkanResourceManager.h"

#include <vector>
#include <vulkan/vulkan_core.h>
#include <memory>

struct FVulkanSemaphore;
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

    virtual FRHITexture* GetBackBufferTexture() const override;
    virtual FRHITextureView* GetBackBufferTextureView() const override;

    inline VkSwapchainKHR GetVkSwapchain() const;
    inline VkExtent2D GetFramebufferExtent() const;
    inline const std::shared_ptr<FVulkanSemaphore>& GetImageAcquiredSemaphore() const;
    inline const std::shared_ptr<FVulkanSemaphore>& GetRenderFinishedSemaphore() const;
    
    VkImage GetImage() const;
    VkImageView GetImageView() const;

private:
    void CreateSwapchain(VkSurfaceKHR Surface, VkSurfaceFormatKHR SurfaceFormat, VkExtent2D Extent);
    void DestroySwapchain();
    bool AcquireNextImageIndex();
    void Present();

private:
    VkSwapchainKHR Swapchain;
    VkExtent2D Extent;

    std::vector<std::unique_ptr<FVulkanTexture>> BackBuffer;
    std::vector<std::unique_ptr<FVulkanTextureView>> BackBufferView;

    //std::vector<VkSemaphore> ImageAcquiredSemaphores;
    //std::vector<VkSemaphore> RenderFinishedSemaphores;

    std::vector<std::shared_ptr<FVulkanSemaphore>> ImageAcquiredSemaphores;
    std::vector<std::shared_ptr<FVulkanSemaphore>> RenderFinishedSemaphores;

    uint32_t CurrentImageIndex;
    uint32_t SemaphoreIndex;
    uint32_t NumAcquireCalls;
    uint32_t NumPresentCalls;

    friend class RVulkanViewport;
};

inline VkSwapchainKHR RVulkanSwapchain::GetVkSwapchain() const
{
    return Swapchain;
}

inline VkExtent2D RVulkanSwapchain::GetFramebufferExtent() const
{
    return Extent;
}

inline const std::shared_ptr<FVulkanSemaphore>& RVulkanSwapchain::GetImageAcquiredSemaphore() const
{
    return ImageAcquiredSemaphores[SemaphoreIndex];
}

inline const std::shared_ptr<FVulkanSemaphore>& RVulkanSwapchain::GetRenderFinishedSemaphore() const
{
    return RenderFinishedSemaphores[CurrentImageIndex];
}