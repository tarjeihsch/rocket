#pragma once

#include "Common/RHISwapchain.h"

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

    inline VkSwapchainKHR GetVkSwapchain() const;
    inline VkExtent2D GetFramebufferExtent() const;

    inline RHITexture* GetBackBufferTexture() const;
    inline std::shared_ptr<VulkanSemaphore> GetReadySemaphore() const;
    inline std::shared_ptr<VulkanSemaphore> GetSignalSemaphore() const;
    
private:
    void CreateSwapchain(VkSurfaceKHR Surface, VkSurfaceFormatKHR SurfaceFormat, VkExtent2D Extent);
    void DestroySwapchain();

    void WaitForImage();
    bool AcquireBackBufferImage();
    void PresentBackBufferImage();

private:
    struct FrameSync
    {
        std::shared_ptr<VulkanSemaphore> ReadySemaphore;
        std::shared_ptr<VulkanSemaphore> SignalSemaphore;
        uint64_t SubmittedTimelineValue = 0;
        uint64_t CompletedTimelineValue = 0;
        bool InFlight = false;
    };
    
    VkSwapchainKHR Swapchain;
    VkExtent2D Extent;

    std::vector<std::unique_ptr<VulkanTexture>> BackBuffers;
    std::vector<FrameSync> FrameSyncData;

    uint32_t CurrentImageIndex;
    uint32_t CurrentFrameIndex;
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

inline std::shared_ptr<VulkanSemaphore> VulkanSwapchain::GetReadySemaphore() const
{
    return FrameSyncData[CurrentFrameIndex].ReadySemaphore;
}

inline std::shared_ptr<VulkanSemaphore> VulkanSwapchain::GetSignalSemaphore() const
{
    return FrameSyncData[CurrentFrameIndex].SignalSemaphore;
}