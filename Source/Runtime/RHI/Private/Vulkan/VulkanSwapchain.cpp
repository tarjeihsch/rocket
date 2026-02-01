#include "VulkanSwapchain.h"

#include "Assertion.h"
#include "Window.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanTexture.h"
#include "VulkanSemaphore.h"

#include <algorithm>

constexpr uint32_t MaxFramesInFlight = 2;

VulkanSwapchain::VulkanSwapchain(VulkanDevice& InDevice)
    : Device(InDevice),
    Swapchain(nullptr),
    CurrentImageIndex(0),
    CurrentFrameIndex(0),
    NumAcquireCalls(0),
    NumPresentCalls(0)
{
}

VulkanSwapchain::~VulkanSwapchain()
{
    DestroySwapchain();
}

void VulkanSwapchain::CreateSwapchain(VkSurfaceKHR Surface, VkSurfaceFormatKHR SurfaceFormat, VkExtent2D InExtent)
{
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VulkanRHI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceCapabilities);

    Extent.width = std::clamp(InExtent.width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
    Extent.height = std::clamp(InExtent.height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);

    // Surface size is fixed (eg. fullscreen)
    if (SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        Extent = SurfaceCapabilities.currentExtent;
    }

    uint32_t ImageCount = SurfaceCapabilities.minImageCount + 1;
    if (SurfaceCapabilities.maxImageCount > 0 && ImageCount > SurfaceCapabilities.maxImageCount)
    {
        ImageCount = SurfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR SwapchainCreateInfo = {};
    SwapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainCreateInfo.surface = Surface;
    SwapchainCreateInfo.minImageCount = ImageCount;
    SwapchainCreateInfo.imageFormat = SurfaceFormat.format;
    SwapchainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
    SwapchainCreateInfo.imageExtent = Extent;
    SwapchainCreateInfo.imageArrayLayers = 1;
    SwapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    SwapchainCreateInfo.preTransform = SurfaceCapabilities.currentTransform;
    SwapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    SwapchainCreateInfo.clipped = VK_TRUE;

    if (GetVulkanRHIMinimal()->RHIGetGraphicsQueueFamilyIndex() != GetVulkanRHIMinimal()->RHIGetPresentQueueFamilyIndex())
    {
        std::vector<uint32_t> QueueFamilyIndices = { GetVulkanRHIMinimal()->RHIGetGraphicsQueueFamilyIndex(), GetVulkanRHIMinimal()->RHIGetPresentQueueFamilyIndex() };

        // Images can be used across multiple queue families without explicit ownership transfers.
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapchainCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(QueueFamilyIndices.size());
        SwapchainCreateInfo.pQueueFamilyIndices = QueueFamilyIndices.data();
    }
    else
    {
        // An image is owned by one queue family at a time and ownership must be explicitly
        // transferred before using it in another queue family. This option offers the best performance.
        SwapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkResult Result = VulkanRHI::vkCreateSwapchainKHR(Device.GetVkDevice(), &SwapchainCreateInfo, nullptr, &Swapchain);
    ensure(Result == VK_SUCCESS, "Failed to create swapchain.");

    RK_LOG_INFO("Swapchain: {} images, {} frames-in-flight", ImageCount, MaxFramesInFlight);

    std::vector<VkImage> Images;

    VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, nullptr);
    Images.resize(ImageCount);
    VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, Images.data());

    for (uint32_t Index = 0; Index < ImageCount; ++Index)
    {
        RHITextureDesc Desc = RHITextureDesc::Create2D(Extent.width, Extent.height, RHIResourceLifetime::Unmanaged, RHITextureFormat::B8G8R8A8_SRGB, RHITextureUsage::Color | RHITextureUsage::CopyDst | RHITextureUsage::Present, RHITextureLayout::None, 0, 0, 1);
        BackBuffers.push_back(std::make_unique<VulkanTexture>(Device, Desc, Images[Index]));
    }

    FrameSyncData.resize(ImageCount, {});

    for (FrameSync& Frame : FrameSyncData)
    {
        Frame.ReadySemaphore = std::make_shared<VulkanSemaphore>();
        Frame.SignalSemaphore = std::make_shared<VulkanSemaphore>();
    }
}

void VulkanSwapchain::DestroySwapchain()
{
    Device.WaitUntilIdle();

    VulkanRHI::vkDestroySwapchainKHR(Device.GetVkDevice(), Swapchain, nullptr);

    BackBuffers.clear();
    FrameSyncData.clear();

    CurrentImageIndex = 0;
    CurrentFrameIndex = 0;
}

void VulkanSwapchain::WaitForImage()
{
    FrameSync Frame = FrameSyncData[CurrentFrameIndex];

    if (Frame.InFlight)
    {
        uint64_t CompletedValue = 0;
        VkResult Result = VulkanRHI::vkGetSemaphoreCounterValue(Device.GetVkDevice(), TimelineSemaphore, &CompletedValue);
        ensure(Result == VK_SUCCESS, "Failed to get timeline semaphore value");
    }
}

bool VulkanSwapchain::AcquireBackBufferImage()
{
    CurrentFrameIndex = (CurrentFrameIndex + 1) % FrameSyncData.size();

    VkResult Result = VulkanRHI::vkAcquireNextImageKHR(Device.GetVkDevice(), Swapchain, UINT64_MAX, static_cast<VkSemaphore>(FrameSyncData[CurrentFrameIndex].ReadySemaphore->GetNativeHandle()), VK_NULL_HANDLE, &CurrentImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        RK_LOG_INFO("Swapchain is out of date .");
        return false;
    }

    ++NumAcquireCalls;
    return true;
}

void VulkanSwapchain::PresentBackBufferImage()
{
    VkSemaphore Semaphore = static_cast<VkSemaphore>(FrameSyncData[CurrentFrameIndex].ReadySemaphore->GetNativeHandle());

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pWaitSemaphores = &Semaphore;
    present_info.waitSemaphoreCount = 1;
    present_info.pSwapchains = &Swapchain;
    present_info.swapchainCount = 1;
    present_info.pImageIndices = &CurrentImageIndex;
    VulkanRHI::vkQueuePresentKHR(GetVulkanRHIMinimal()->RHIGetPresentVkQueue(), &present_info);
 
    ++NumPresentCalls;
}

RHITexture* VulkanSwapchain::GetBackBufferTexture() const
{
    ensure(BackBuffers.size() != 0)

    return BackBuffers[CurrentImageIndex].get();
}