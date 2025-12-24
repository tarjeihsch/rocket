#if defined(_WIN32)
    #ifndef VK_USE_PLATFORM_WIN32_KHR
        #define VK_USE_PLATFORM_WIN32_KHR
    #endif
#endif

#if defined(__linux__)
    #ifndef VK_USE_PLATFORM_XLIB_KHR
        #define VK_USE_PLATFORM_XLIB_KHR
    #endif
    #ifndef VK_USE_PLATFORM_WAYLAND_KHR
        #define VK_USE_PLATFORM_WAYLAND_KHR
    #endif
#endif

#include "VulkanSwapchain.h"

#include "Assertion.h"
#include "Platform.h"
#include "Window.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanTexture.h"

#include <algorithm>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <wayland-client-core.h>
#elif defined(__APPLE__)
    #include <objc/objc.h>
#endif

RVulkanSwapchain::RVulkanSwapchain(FVulkanDevice& InDevice, RRHISwapchainDesc& InDesc)
    : Device(InDevice),
    Swapchain(nullptr),
    Surface(nullptr),
    CurrentImageIndex(0),
    SemaphoreIndex(0),
    NumAcquireCalls(0),
    NumPresentCalls(0)
{
    VkResult Result;

    switch (InDesc.Backend)
    {
        case ENativePlatformBackend::Win32:
        {
#if defined(_WIN32)
            auto ptr = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(VulkanRHI::vkGetInstanceProcAddr(GetVulkanRHIMinimal()->RHIGetVkInstance(), "vkCreateWin32SurfaceKHR"));
            ensure(ptr != nullptr, "vkCreateWin32SurfaceKHR not available. Did you enable VK_KHR_win32_surface?");

            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hwnd = static_cast<HWND>(InDesc.NativeWindowHandle);
            surfaceCreateInfo.hinstance = static_cast<HINSTANCE>(InDesc.NativeInstanceHandle);

            Result = ptr(GetVulkanRHIMinimal()->RHIGetVkInstance(), &surfaceCreateInfo, nullptr, &Surface);
#endif
            break;
        }

        case ENativePlatformBackend::X11:
        {
#if defined(__linux__)
            auto ptr = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(VulkanRHI::vkGetInstanceProcAddr(GetVulkanRHIMinimal()->RHIGetVkInstance(), "vkCreateXlibSurfaceKHR"));
            ensure(ptr != nullptr, "vkCreateXlibSurfaceKHR not available. Did you enable VK_KHR_xlib_surface?");

            VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.dpy = static_cast<Display*>(windowInfo.display);
            surfaceCreateInfo.window = static_cast<::Window>(reinterpret_cast<uintptr_t>(windowInfo.instance));

            Result = ptr(GetVulkanRHIMinimal()->RHIGetVkInstance(), &surfaceCreateInfo, nullptr, &Surface);
#endif
            break;
        }
        case ENativePlatformBackend::Wayland:
        {
#if defined(__linux__)
            auto ptr = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(VulkanRHI::vkGetInstanceProcAddr(GetVulkanRHIMinimal()->RHIGetVkInstance(), "vkCreateWaylandSurfaceKHR"));
            ensure(ptr != nullptr, "vkCreateWaylandSurfaceKHR not available. Did you enable VK_KHR_wayland_surface?");

            VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.display = static_cast<wl_display*>(windowInfo.display);
            surfaceCreateInfo.surface = static_cast<wl_surface*>(windowInfo.instance);

            Result = ptr(GetVulkanRHIMinimal()->RHIGetVkInstance(), &surfaceCreateInfo, nullptr, &Surface);
#endif
            break;
        }
        case ENativePlatformBackend::Cocoa:
        {
#if defined(__APPLE__)
            // IMPORTANT:
            // Vulkan surface creation on macOS is normally done via VK_EXT_metal_surface using a CAMetalLayer*.
            // GLFW's "native cocoa window" (NSWindow*) is not directly used by Vulkan WSI.
            ensure(false, "Cocoa surface creation requires VK_EXT_metal_surface with a CAMetalLayer*. Provide layer handle via WindowInfo.");

            Result = VK_ERROR_EXTENSION_NOT_PRESENT;
#endif
            break;
        }
    }

    ensure(Result == VK_SUCCESS, "Failed to create Vulkan surface.");

    uint32_t SurfaceFormatCount;
    VulkanRHI::vkGetPhysicalDeviceSurfaceFormatsKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceFormatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
    if (SurfaceFormatCount)
    {
        SurfaceFormats.resize(SurfaceFormatCount);
        VulkanRHI::vkGetPhysicalDeviceSurfaceFormatsKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceFormatCount, SurfaceFormats.data());
    }

    for (const auto& Element : SurfaceFormats)
    {
        if (Element.format == VK_FORMAT_B8G8R8A8_SRGB && Element.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SurfaceFormat = Element;
        }
    }

    uint32_t PresentModeCount;
    VulkanRHI::vkGetPhysicalDeviceSurfacePresentModesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &PresentModeCount, nullptr);

    std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
    if (PresentModeCount)
    {
        PresentModes.resize(PresentModeCount);
        VulkanRHI::vkGetPhysicalDeviceSurfacePresentModesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &PresentModeCount, PresentModes.data());
    }

    for (const auto& Element : PresentModes)
    {
        if (Element == VK_PRESENT_MODE_FIFO_KHR)
        {
            PresentMode = Element;
        }
    }

    CreateSwapchain(InDesc.Size);
    AcquireNextImageIndex();
}

RVulkanSwapchain::~RVulkanSwapchain()
{
    DestroySwapchain();

    VulkanRHI::vkDestroySurfaceKHR(GetVulkanRHIMinimal()->RHIGetVkInstance(), Surface, nullptr);
}

void RVulkanSwapchain::CreateSwapchain(Vector2i Size)
{
    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VulkanRHI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetVulkanRHIMinimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceCapabilities);

    const uint32_t Width = static_cast<uint32_t>(Size.x);
    const uint32_t Height = static_cast<uint32_t>(Size.y);

    Extent = { Width, Height };
    Extent.width = std::clamp(Extent.width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width);
    Extent.height = std::clamp(Extent.height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height);

    // Surface size is fixed (eg. fullscreen)
    if (SurfaceCapabilities.currentExtent.width != UINT32_MAX)
    {
        Extent = SurfaceCapabilities.currentExtent;
    }

    uint32_t ImageCount = SurfaceCapabilities.minImageCount;
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
    std::vector<VkImageView> Views;

    VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, nullptr);
    Images.resize(ImageCount);
    Views.resize(ImageCount);
    VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), Swapchain, &ImageCount, Images.data());

    for (uint32_t Index = 0; Index < ImageCount; ++Index)
    {
        FRHITextureDesc Desc = FRHITextureDesc::Create2D(Extent.width, Extent.height, ERHIResourceLifetime::Unmanaged, ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureUsage::Color | ERHITextureUsage::CopyDst | ERHITextureUsage::Present, ERHITextureLayout::Undefined, 0, 0, 1);
        FRHITextureViewDesc ViewDesc = FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color, ERHIResourceLifetime::Persistent);
        Backbuffer.push_back(std::make_unique<FVulkanTexture>(Device, Desc, Images[Index]));
        BackbufferView.push_back(std::make_unique<FVulkanTextureView>(Device, ViewDesc, Backbuffer.back().get()));
    }

    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    ImageAcquiredSemaphores.resize(ImageCount - 1);
    RenderFinishedSemaphores.resize(ImageCount);

    for (uint32_t Index = 0; Index < ImageAcquiredSemaphores.size(); ++Index)
    {
        VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &ImageAcquiredSemaphores[Index]);
    }

    for (uint32_t Index = 0; Index < RenderFinishedSemaphores.size(); ++Index)
    {
        VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &RenderFinishedSemaphores[Index]);
    }
}

bool RVulkanSwapchain::AcquireNextImageIndex()
{
    SemaphoreIndex = (SemaphoreIndex + 1) % ImageAcquiredSemaphores.size();

    VkResult Result = VulkanRHI::vkAcquireNextImageKHR(GetVulkanRHIMinimal()->RHIGetVkDevice(), Swapchain, UINT64_MAX, ImageAcquiredSemaphores[SemaphoreIndex], VK_NULL_HANDLE, &CurrentImageIndex);

    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
    {
        RK_LOG_INFO("Swapchain is out of date.");

        DestroySwapchain();
        CreateSwapchain(Vector2i(1280, 720));
		AcquireNextImageIndex();
    }

    ++NumAcquireCalls;
    return true;
}

void RVulkanSwapchain::Present()
{
    VkPresentInfoKHR PresentInfo = {};
    PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInfo.pNext = VK_NULL_HANDLE;
    PresentInfo.pSwapchains = &Swapchain;
    PresentInfo.swapchainCount = 1;
    PresentInfo.pWaitSemaphores = &RenderFinishedSemaphores[CurrentImageIndex];
    PresentInfo.waitSemaphoreCount = 1;
    PresentInfo.pImageIndices = &CurrentImageIndex;

    VulkanRHI::vkQueuePresentKHR(GetVulkanRHIMinimal()->RHIGetPresentVkQueue(), &PresentInfo);

    ++NumPresentCalls;
}

void RVulkanSwapchain::DestroySwapchain()
{
    Device.WaitUntilIdle();

    VulkanRHI::vkDestroySwapchainKHR(Device.GetVkDevice(), Swapchain, nullptr);

    for (auto& Semaphore : ImageAcquiredSemaphores)
    {
        VulkanRHI::vkDestroySemaphore(Device.GetVkDevice(), Semaphore, nullptr);
    }

    for (auto& Semaphore : RenderFinishedSemaphores)
    {
        VulkanRHI::vkDestroySemaphore(Device.GetVkDevice(), Semaphore, nullptr);
    }

    Backbuffer.clear();
    BackbufferView.clear();
    ImageAcquiredSemaphores.clear();
    RenderFinishedSemaphores.clear();

    CurrentImageIndex = UINT32_MAX;
    SemaphoreIndex = 0;
    NumAcquireCalls = 0;
    NumPresentCalls = 0;
}

FRHITexture* RVulkanSwapchain::GetBackBufferTexture() const
{
    ensure(Backbuffer.size() != 0)

    return Backbuffer[CurrentImageIndex].get();
}

FRHITextureView* RVulkanSwapchain::GetBackBufferTextureView() const
{
    ensure(BackbufferView.size() != 0)

    return BackbufferView[CurrentImageIndex].get();
}

VkImage RVulkanSwapchain::GetImage() const
{
	return static_cast<VkImage>(GetBackBufferTexture()->GetHandle());
}

VkImageView RVulkanSwapchain::GetImageView() const
{
	return static_cast<VkImageView>(GetBackBufferTextureView()->GetHandle());
}
