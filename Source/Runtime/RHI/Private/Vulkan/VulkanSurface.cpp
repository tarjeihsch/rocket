#include "VulkanSurface.h"

#include "Assertion.h"
#include "VulkanLoader.h"
#include "VulkanSemaphore.h"
#include "VulkanSwapchain.h"
#include "Vulkan/VulkanRHIMinimal.h"

#if defined(_WIN32)
    #include <windows.h>
    #include <vulkan/vulkan_win32.h>
#elif defined(__linux__)
    #include <X11/Xlib.h>
    #include <vulkan/vulkan_xlib.h>
    #include <wayland-client-core.h>
    #include <vulkan/vulkan_wayland.h>
#elif defined(__APPLE__)
    #include <objc/objc.h>
#endif

VulkanSurface::VulkanSurface(VulkanDevice& InDevice, RHISurfaceDesc& InDesc)
    : Device(InDevice)
{
    VkResult Result;

    switch (InDesc.Backend)
    {
        case ENativePlatformBackend::Win32:
        {
#if defined(_WIN32)
            auto ptr = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(VulkanRHI::vkGetInstanceProcAddr(get_vulkan_rhi_minimal()->RHIGetVkInstance(), "vkCreateWin32SurfaceKHR"));
            ensure(ptr != nullptr, "vkCreateWin32SurfaceKHR not available. Did you enable VK_KHR_win32_surface?");

            VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.hwnd = static_cast<HWND>(InDesc.NativeWindowHandle);
            surfaceCreateInfo.hinstance = static_cast<HINSTANCE>(InDesc.NativeInstanceHandle);

            Result = ptr(get_vulkan_rhi_minimal()->RHIGetVkInstance(), &surfaceCreateInfo, nullptr, &Surface);
#endif
            break;
        }

        case ENativePlatformBackend::X11:
        {
#if defined(__linux__)
            auto ptr = reinterpret_cast<PFN_vkCreateXlibSurfaceKHR>(VulkanRHI::vkGetInstanceProcAddr(get_vulkan_rhi_minimal()->RHIGetVkInstance(), "vkCreateXlibSurfaceKHR"));
            ensure(ptr != nullptr, "vkCreateXlibSurfaceKHR not available. Did you enable VK_KHR_xlib_surface?");

            VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.dpy = static_cast<Display*>(InDesc.NativeInstanceHandle);
            surfaceCreateInfo.window = reinterpret_cast<uintptr_t>(InDesc.NativeWindowHandle);

            Result = ptr(get_vulkan_rhi_minimal()->RHIGetVkInstance(), &surfaceCreateInfo, nullptr, &Surface);
#endif
            break;
        }
        case ENativePlatformBackend::Wayland:
        {
#if defined(__linux__)
            auto ptr = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(VulkanRHI::vkGetInstanceProcAddr(get_vulkan_rhi_minimal()->RHIGetVkInstance(), "vkCreateWaylandSurfaceKHR"));
            ensure(ptr != nullptr, "vkCreateWaylandSurfaceKHR not available. Did you enable VK_KHR_wayland_surface?");

            VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo = {};
            surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
            surfaceCreateInfo.display = static_cast<wl_display*>(InDesc.NativeWindowHandle);
            surfaceCreateInfo.surface = static_cast<wl_surface*>(InDesc.NativeInstanceHandle);

            Result = ptr(get_vulkan_rhi_minimal()->RHIGetVkInstance(), &surfaceCreateInfo, nullptr, &Surface);
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
    ensure(Surface != VK_NULL_HANDLE, "Failed to create Vulkan surface.");

    uint32_t SurfaceFormatCount;
    VulkanRHI::vkGetPhysicalDeviceSurfaceFormatsKHR(get_vulkan_rhi_minimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceFormatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> SurfaceFormats(SurfaceFormatCount);
    if (SurfaceFormatCount)
    {
        SurfaceFormats.resize(SurfaceFormatCount);
        VulkanRHI::vkGetPhysicalDeviceSurfaceFormatsKHR(get_vulkan_rhi_minimal()->RHIGetVkPhysicalDevice(), Surface, &SurfaceFormatCount, SurfaceFormats.data());
    }

    for (const auto& Element : SurfaceFormats)
    {
        if (Element.format == VK_FORMAT_B8G8R8A8_SRGB && Element.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            SurfaceFormat = Element;
        }
    }

    uint32_t PresentModeCount;
    VulkanRHI::vkGetPhysicalDeviceSurfacePresentModesKHR(get_vulkan_rhi_minimal()->RHIGetVkPhysicalDevice(), Surface, &PresentModeCount, nullptr);

    std::vector<VkPresentModeKHR> PresentModes(PresentModeCount);
    if (PresentModeCount)
    {
        PresentModes.resize(PresentModeCount);
        VulkanRHI::vkGetPhysicalDeviceSurfacePresentModesKHR(get_vulkan_rhi_minimal()->RHIGetVkPhysicalDevice(), Surface, &PresentModeCount, PresentModes.data());
    }

    for (const auto& Element : PresentModes)
    {
        if (Element == VK_PRESENT_MODE_FIFO_KHR)
        {
            PresentMode = Element;
        }
    }

    Swapchain = new VulkanSwapchain(Device);
    Swapchain->CreateSwapchain(Surface, SurfaceFormat, {1280, 720});
}

VulkanSurface::~VulkanSurface()
{
    delete Swapchain;

    VulkanRHI::vkDestroySurfaceKHR(get_vulkan_rhi_minimal()->RHIGetVkInstance(), Surface, nullptr);
}

RHITexture* VulkanSurface::get_back_buffer_texture()
{
    return Swapchain->GetBackBufferTexture();
}

void VulkanSurface::acquire(RHICommandListContext& ctx)
{
    if (!Swapchain->acquire_back_buffer_image((VulkanCommandListContext&)ctx))
    {
        Swapchain->DestroySwapchain();
        Swapchain->CreateSwapchain(Surface, SurfaceFormat, {1280, 720});
        Swapchain->acquire_back_buffer_image((VulkanCommandListContext&)ctx);
    }
}

void VulkanSurface::present(RHICommandListContext& ctx)
{
    Swapchain->present_back_buffer_image((VulkanCommandListContext&)ctx);
}

std::shared_ptr<RHISemaphore> VulkanSurface::GetImageAvailableSemaphore() const
{
    return Swapchain->get_image_acquired_semaphore();
}

std::shared_ptr<RHISemaphore> VulkanSurface::GetRenderFinishedSemaphore() const
{
    return Swapchain->get_render_finished_semaphore();
}