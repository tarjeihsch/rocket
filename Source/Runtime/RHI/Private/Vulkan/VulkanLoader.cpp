#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanLoader.h"

#include "Vulkan/VulkanRHIMinimal.h"

#include <assert.h>

#ifdef RK_PLATFORM_WINDOWS
    #include <windows.h>
    #define VK_DLSYM(lib, name) GetProcAddress((HMODULE)(lib), name)
#endif

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #define VK_DLSYM(lib, name) dlsym(lib, name)
#endif

namespace VulkanRHI
{
    void* Library = nullptr;

    #define DEFINE_PFN(name) PFN_##name name = nullptr;
    VULKAN_ALL_FUNCTIONS(DEFINE_PFN)
    #undef DEFINE_PFN
};

VkImageCreateInfo VulkanRHI::GetVkImageCreateInfo(VkExtent3D Extent, VkFormat Format, VkImageUsageFlags UsageFlags)
{
    VkImageCreateInfo ImageCreateInfo = {};
    ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.extent = Extent;
    ImageCreateInfo.format = Format;
    ImageCreateInfo.mipLevels = 1;
    ImageCreateInfo.arrayLayers = 1;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.usage = UsageFlags;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return ImageCreateInfo;
}

VkImageViewCreateInfo VulkanRHI::GetVkImageViewCreateInfo(VkImage Image, VkFormat Format, VkImageAspectFlags AspectFlags)
{
    VkImageViewCreateInfo ImageViewCreateInfo = {};
    ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ImageViewCreateInfo.image = Image;
    ImageViewCreateInfo.format = Format;
    ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInfo.subresourceRange.levelCount = 1;
    ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInfo.subresourceRange.layerCount = 1;
    ImageViewCreateInfo.subresourceRange.aspectMask = AspectFlags; // VK_IMAGE_ASPECT_COLOR_BIT

    return ImageViewCreateInfo;
}

VkImageUsageFlags VulkanRHI::RHITextureUsageToVkImageUsageFlags(ERHITextureUsage Usage)
{
    VkImageUsageFlags Flags = 0;

    if ((Usage & ERHITextureUsage::Color) == ERHITextureUsage::Color)
    {
        Flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    if ((Usage & ERHITextureUsage::Depth) == ERHITextureUsage::Depth || (Usage & ERHITextureUsage::Stencil) == ERHITextureUsage::Stencil)
    {
        Flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }

    if ((Usage & ERHITextureUsage::CopySrc) == ERHITextureUsage::CopySrc)
    {
        Flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    if ((Usage & ERHITextureUsage::CopyDst) == ERHITextureUsage::CopyDst)
    {
        Flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    if ((Usage & ERHITextureUsage::SRV) == ERHITextureUsage::SRV)
    {
        Flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }

    if ((Usage & ERHITextureUsage::UAV) == ERHITextureUsage::UAV)
    {
        Flags |= VK_IMAGE_USAGE_STORAGE_BIT;
    }

    // Swapchain images (Present) are not user-created, but we include this for completeness.
    if ((Usage & ERHITextureUsage::Present) == ERHITextureUsage::Present)
    {
        // Swapchain images usually come with COLOR_ATTACHMENT | TRANSFER_DST
        Flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        Flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    return Flags;
}

VkImageLayout VulkanRHI::RHITextureUsageToVkImageLayout(ERHITextureUsage Usage)
{
    if ((Usage & ERHITextureUsage::Present) == ERHITextureUsage::Present)
    {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    if ((Usage & ERHITextureUsage::Color) == ERHITextureUsage::Color)
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    if ((Usage & ERHITextureUsage::Depth) == ERHITextureUsage::Depth || (Usage & ERHITextureUsage::Stencil) == ERHITextureUsage::Stencil)
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    if ((Usage & ERHITextureUsage::UAV) == ERHITextureUsage::UAV)
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    if ((Usage & ERHITextureUsage::SRV) == ERHITextureUsage::SRV)
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    if ((Usage & ERHITextureUsage::CopyDst) == ERHITextureUsage::CopyDst)
    {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }

    if ((Usage & ERHITextureUsage::CopySrc) == ERHITextureUsage::CopySrc)
    {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }

    return VK_IMAGE_LAYOUT_UNDEFINED; // Fallback or initialization
}

void LoadVulkanLibrary()
{
#if RK_PLATFORM_WINDOWS
    VulkanRHI::Library = LoadLibraryA("vulkan-1.dll");
    VulkanRHI::vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(static_cast<HMODULE>(VulkanRHI::Library), "vkGetInstanceProcAddr"));
#endif

#if RK_PLATFORM_LINUX
    VulkanRHI::Library = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    VulkanRHI::vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(VulkanRHI::Library, "vkGetInstanceProcAddr"));
#endif

    assert(VulkanRHI::Library && "Unable to find and load Vulkan dynamic library.");
}

void LoadVulkanGlobalFunc()
{
    #define LOAD_GLOBAL_FN(Name) VulkanRHI::Name = reinterpret_cast<PFN_##Name>(VulkanRHI::vkGetInstanceProcAddr(nullptr, #Name));
    VULKAN_GLOBAL_FUNCTIONS(LOAD_GLOBAL_FN)
    #undef LOAD_GLOBAL_FN
}

void LoadVulkanInstanceFunc(VkInstance Instance)
{
    assert(VulkanRHI::vkGetInstanceProcAddr);

    #define LOAD_INSTANCE_FN(Name) VulkanRHI::Name = reinterpret_cast<PFN_##Name>(VulkanRHI::vkGetInstanceProcAddr(Instance, #Name));
    VULKAN_INSTANCE_FUNCTIONS(LOAD_INSTANCE_FN);
    #undef LOAD_INSTANCE_FN
}

void LoadVulkanDeviceFunc(VkDevice Device)
{
    assert(VulkanRHI::vkGetDeviceProcAddr);

    #define LOAD_DEVICE_FN(Name) VulkanRHI::Name = reinterpret_cast<PFN_##Name>(VulkanRHI::vkGetDeviceProcAddr(Device, #Name));
    VULKAN_DEVICE_FUNCTIONS(LOAD_DEVICE_FN);
    #undef LOAD_DEVICE_FN
}

VmaVulkanFunctions CreateVmaFuncTable()
{
    VmaVulkanFunctions Funcs = {};

    #define ASSIGN_VMA(name) Funcs.name = VulkanRHI::name;
    VMA_BASE_FUNCTIONS(ASSIGN_VMA)
    #undef ASSIGN_VMA

    Funcs.vkGetBufferMemoryRequirements2KHR = VulkanRHI::vkGetBufferMemoryRequirements2;
    Funcs.vkGetImageMemoryRequirements2KHR = VulkanRHI::vkGetImageMemoryRequirements2;
    Funcs.vkBindBufferMemory2KHR = VulkanRHI::vkBindBufferMemory2;
    Funcs.vkBindImageMemory2KHR = VulkanRHI::vkBindImageMemory2;
    Funcs.vkGetPhysicalDeviceMemoryProperties2KHR = VulkanRHI::vkGetPhysicalDeviceMemoryProperties2;
    Funcs.vkGetDeviceBufferMemoryRequirements = VulkanRHI::vkGetDeviceBufferMemoryRequirements;
    Funcs.vkGetDeviceImageMemoryRequirements = VulkanRHI::vkGetDeviceImageMemoryRequirements;

    return Funcs;
}

void FreeVulkanLibrary()
{
#if RK_PLATFORM_LINUX
    dlclose(VulkanRHI::Library);
    VulkanRHI::Library = nullptr;
#endif
}
