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

VkImageLayout GetVulkanLayout(ERHITextureLayout layout)
{
    switch (layout)
    {
    case ERHITextureLayout::Undefined:         return VK_IMAGE_LAYOUT_UNDEFINED;
    case ERHITextureLayout::General:           return VK_IMAGE_LAYOUT_GENERAL;
    case ERHITextureLayout::ColorAttachment:   return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ERHITextureLayout::DepthStencil:      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ERHITextureLayout::DepthReadOnly:     return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ERHITextureLayout::ShaderReadOnly:    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ERHITextureLayout::TransferSrc:       return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ERHITextureLayout::TransferDst:       return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ERHITextureLayout::Present:           return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:                                    return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

template<typename TEnum>
inline bool EnumHasAnyFlags(TEnum Value, TEnum Flags)
{
    using T = std::underlying_type_t<TEnum>;
    return (static_cast<T>(Value) & static_cast<T>(Flags)) != 0;
}

VkAccessFlags VulkanRHI::GetVulkanAccess(ERHIResourceAccess access)
{
    VkAccessFlags flags = 0;

    if (EnumHasAnyFlags(access, ERHIResourceAccess::ShaderSampledRead)) flags |= VK_ACCESS_SHADER_READ_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::ShaderStorageRead)) flags |= VK_ACCESS_SHADER_READ_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::ShaderStorageWrite)) flags |= VK_ACCESS_SHADER_WRITE_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::InputAttachmentRead)) flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::TransferRead)) flags |= VK_ACCESS_TRANSFER_READ_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::TransferWrite)) flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::ColorAttachmentWrite)) flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::DepthStencilAttachmentWrite)) flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::HostRead)) flags |= VK_ACCESS_HOST_READ_BIT;
    if (EnumHasAnyFlags(access, ERHIResourceAccess::HostWrite)) flags |= VK_ACCESS_HOST_WRITE_BIT;

    return flags;
}


VkImageLayout VulkanRHI::GetVulkanLayout(ERHITextureLayout layout)
{
    switch (layout)
    {
    case ERHITextureLayout::Undefined:         return VK_IMAGE_LAYOUT_UNDEFINED;
    case ERHITextureLayout::General:           return VK_IMAGE_LAYOUT_GENERAL;
    case ERHITextureLayout::ColorAttachment:   return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ERHITextureLayout::DepthStencil:      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ERHITextureLayout::DepthReadOnly:     return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case ERHITextureLayout::ShaderReadOnly:    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case ERHITextureLayout::TransferSrc:       return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ERHITextureLayout::TransferDst:       return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ERHITextureLayout::Present:           return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:                                   return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

VkPipelineStageFlags VulkanRHI::GetVulkanPipelineStage(ERHIPipelineStage stage)
{
    VkPipelineStageFlags flags = 0;

    if (EnumHasAnyFlags(stage, ERHIPipelineStage::TopOfPipe))       flags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    if (EnumHasAnyFlags(stage, ERHIPipelineStage::VertexShader))    flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if (EnumHasAnyFlags(stage, ERHIPipelineStage::FragmentShader))  flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if (EnumHasAnyFlags(stage, ERHIPipelineStage::ComputeShader))   flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    if (EnumHasAnyFlags(stage, ERHIPipelineStage::Transfer))        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    if (EnumHasAnyFlags(stage, ERHIPipelineStage::ColorAttachment)) flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (EnumHasAnyFlags(stage, ERHIPipelineStage::DepthStencil))    flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                                                            VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    if (EnumHasAnyFlags(stage, ERHIPipelineStage::BottomOfPipe))    flags |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    return flags;
}

VkImageUsageFlags VulkanRHI::GetVulkanUsage(ERHITextureUsage usage)
{
    VkImageUsageFlags flags = 0;

    if (EnumHasAnyFlags(usage, ERHITextureUsage::Present))  flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Used indirectly
    if (EnumHasAnyFlags(usage, ERHITextureUsage::Color))    flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (EnumHasAnyFlags(usage, ERHITextureUsage::Depth))    flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (EnumHasAnyFlags(usage, ERHITextureUsage::Stencil))  flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (EnumHasAnyFlags(usage, ERHITextureUsage::CopySrc))  flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (EnumHasAnyFlags(usage, ERHITextureUsage::CopyDst))  flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (EnumHasAnyFlags(usage, ERHITextureUsage::SRV))      flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (EnumHasAnyFlags(usage, ERHITextureUsage::UAV))      flags |= VK_IMAGE_USAGE_STORAGE_BIT;

    return flags;
}

VkFormat VulkanRHI::GetVkFormat(ERHITextureFormat Format)
{
    if (Format == ERHITextureFormat::B8G8R8A8_SRGB) { return VK_FORMAT_B8G8R8A8_SRGB; }

    return VK_FORMAT_UNDEFINED;
}

VkImageAspectFlags VulkanRHI::GetVkAspect(ERHITextureAspect Aspect)
{
    if (Aspect == ERHITextureAspect::Color) { return VK_IMAGE_ASPECT_COLOR_BIT; }

    return VK_IMAGE_ASPECT_NONE;
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
