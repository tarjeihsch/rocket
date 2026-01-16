#include "VulkanSwapchain.h"

#include "Assertion.h"
#include "Window.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanTexture.h"

#include <algorithm>

#include "VulkanSemaphore.h"

VulkanSwapchain::VulkanSwapchain(VulkanDevice& InDevice)
    : Device(InDevice),
    Swapchain(nullptr),
    CurrentImageIndex(0),
    SemaphoreIndex(0),
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
    // Cache the current extent
    Extent = InExtent;

    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VulkanRHI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceCapabilities);

    Extent.width = std::clamp(Extent.width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
    Extent.height = std::clamp(Extent.height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);

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

    RK_LOG_INFO("Swapchain: {} images, {} frames-in-flight", ImageCount, ImageCount - 1);

    std::vector<VkImage> Images;

    VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, nullptr);
    Images.resize(ImageCount);
    VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, Images.data());

    for (uint32_t Index = 0; Index < ImageCount; ++Index)
    {
        RHITextureDesc Desc = RHITextureDesc::Create2D(Extent.width, Extent.height, RHIResourceLifetime::Unmanaged, RHITextureFormat::B8G8R8A8_SRGB, RHITextureUsage::Color | RHITextureUsage::CopyDst | RHITextureUsage::Present, RHITextureLayout::None, 0, 0, 1);
        BackBuffers.push_back(std::make_unique<VulkanTexture>(Device, Desc, Images[Index]));
    }

    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    ImageAcquiredSemaphores.resize(ImageCount - 1);
    RenderFinishedSemaphores.resize(ImageCount);

    for (uint32_t Index = 0; Index < ImageAcquiredSemaphores.size(); ++Index)
    {
        VkSemaphore Semaphore;
        VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &Semaphore);

        ImageAcquiredSemaphores[Index] = std::make_shared<VulkanSemaphore>(Semaphore);
    }

    for (uint32_t Index = 0; Index < RenderFinishedSemaphores.size(); ++Index)
    {
        VkSemaphore Semaphore;
        VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &Semaphore);

        RenderFinishedSemaphores[Index] = std::make_shared<VulkanSemaphore>(Semaphore);
    }

    ensure(ImageAcquiredSemaphores.size() > 0, "Swapchain has no acquire semaphores (image count too low?)");
}

void VulkanSwapchain::DestroySwapchain()
{
    Device.WaitUntilIdle();

    if (Swapchain != VK_NULL_HANDLE)
    {
        VulkanRHI::vkDestroySwapchainKHR(Device.GetVkDevice(), Swapchain, nullptr);
        Swapchain = VK_NULL_HANDLE;
    }

    // TODO: Use DTOR to release/destroy
    for (auto& Semaphore : ImageAcquiredSemaphores)
    {
        VulkanRHI::vkDestroySemaphore(Device.GetVkDevice(), Semaphore->Handle, nullptr);
    }

    // TODO: Use DTOR to release/destroy
    for (auto& Semaphore : RenderFinishedSemaphores)
    {
        VulkanRHI::vkDestroySemaphore(Device.GetVkDevice(), Semaphore->Handle, nullptr);
    }

    BackBuffers.clear();
    ImageAcquiredSemaphores.clear();
    RenderFinishedSemaphores.clear();

    CurrentImageIndex = UINT32_MAX;
    SemaphoreIndex = 0;
    NumAcquireCalls = 0;
    NumPresentCalls = 0;
}

bool VulkanSwapchain::AcquireBackBufferImage()
{
    SemaphoreIndex = (SemaphoreIndex + 1) % ImageAcquiredSemaphores.size();

    VkResult Result = VulkanRHI::vkAcquireNextImageKHR(GetVulkanRHIMinimal()->RHIGetVkDevice(), Swapchain, UINT64_MAX, ImageAcquiredSemaphores[SemaphoreIndex]->Handle, VK_NULL_HANDLE, &CurrentImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        RK_LOG_INFO("Swapchain is out of date.");
        return false;
    }

    ++NumAcquireCalls;
    return true;
}

void VulkanSwapchain::PresentBackBufferImage()
{
    ++NumPresentCalls;
}

RHITexture* VulkanSwapchain::GetBackBufferTexture() const
{
    ensure(BackBuffers.size() != 0)

    return BackBuffers[CurrentImageIndex].get();
}