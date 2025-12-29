#include "VulkanSwapchain.h"

#include "Assertion.h"
#include "Window.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanTexture.h"

#include <algorithm>

#include "VulkanCommandList.h"
#include "VulkanSemaphore.h"

VulkanSwapchain::VulkanSwapchain(FVulkanDevice& InDevice)
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
        FRHITextureDesc Desc = FRHITextureDesc::Create2D(Extent.width, Extent.height, ERHIResourceLifetime::Unmanaged, ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureUsage::Color | ERHITextureUsage::CopyDst | ERHITextureUsage::Present, ERHITextureLayout::Undefined, 0, 0, 1);
        FRHITextureViewDesc ViewDesc = FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color, ERHIResourceLifetime::Persistent);
        BackBuffers.push_back(std::make_unique<FVulkanTexture>(Device, Desc, Images[Index]));
    }

    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    ImageAcquiredSemaphores.resize(ImageCount - 1);
    RenderFinishedSemaphores.resize(ImageCount);

    for (uint32_t Index = 0; Index < ImageAcquiredSemaphores.size(); ++Index)
    {
        VkSemaphore Semaphore;
        VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &Semaphore);

        ImageAcquiredSemaphores[Index] = std::make_shared<FVulkanSemaphore>(Semaphore);
    }

    for (uint32_t Index = 0; Index < RenderFinishedSemaphores.size(); ++Index)
    {
        VkSemaphore Semaphore;
        VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &Semaphore);

        RenderFinishedSemaphores[Index] = std::make_shared<FVulkanSemaphore>(Semaphore);
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

bool VulkanSwapchain::acquire_back_buffer_image(VulkanCommandListContext& ctx)
{
    SemaphoreIndex = (SemaphoreIndex + 1) % ImageAcquiredSemaphores.size();

    VkResult Result = VulkanRHI::vkAcquireNextImageKHR(GetVulkanRHIMinimal()->RHIGetVkDevice(), Swapchain, UINT64_MAX, ImageAcquiredSemaphores[SemaphoreIndex]->Handle, VK_NULL_HANDLE, &CurrentImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        RK_LOG_INFO("Swapchain is out of date.");
        return false;
    }

    ctx.add_wait_semaphore(get_image_acquired_semaphore()->Handle, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    ++NumAcquireCalls;
    return true;
}

void VulkanSwapchain::present_back_buffer_image(VulkanCommandListContext& ctx)
{
    ctx.add_signal_semaphore(get_render_finished_semaphore()->Handle, VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT);

    ctx.add_post_submit_event([this]()
    {
        VkPresentInfoKHR PresentInfo = {};
        PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        PresentInfo.pNext = VK_NULL_HANDLE;
        PresentInfo.pSwapchains = &Swapchain;
        PresentInfo.swapchainCount = 1;
        PresentInfo.pWaitSemaphores = &RenderFinishedSemaphores[CurrentImageIndex]->Handle;
        PresentInfo.waitSemaphoreCount = 1;
        PresentInfo.pImageIndices = &CurrentImageIndex;

        VulkanRHI::vkQueuePresentKHR(GetVulkanRHIMinimal()->RHIGetPresentVkQueue(), &PresentInfo);
        ++NumPresentCalls;
    });
}

RHITexture* VulkanSwapchain::GetBackBufferTexture() const
{
    ensure(BackBuffers.size() != 0)

    return BackBuffers[CurrentImageIndex].get();
}