#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanLoader.h"

#include "Common/RHIBuffer.h"
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
    ImageViewCreateInfo.subresourceRange.aspectMask = AspectFlags;
    return ImageViewCreateInfo;
}

VkBufferCreateInfo VulkanRHI::GetVkBufferCreateInfo(uint64_t Size, VkBufferUsageFlags UsageFlags)
{
    VkBufferCreateInfo BufferCreateInfo = {};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.size = Size;
    BufferCreateInfo.usage = UsageFlags;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    return BufferCreateInfo;
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

VkBlendFactor VulkanRHI::GetVkBlendFactor(ERHIBlendFactor Factor)
{
    switch (Factor)
    {
    case ERHIBlendFactor::Zero:                  return VK_BLEND_FACTOR_ZERO;
    case ERHIBlendFactor::One:                   return VK_BLEND_FACTOR_ONE;
    case ERHIBlendFactor::SrcColor:              return VK_BLEND_FACTOR_SRC_COLOR;
    case ERHIBlendFactor::OneMinusSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case ERHIBlendFactor::DstColor:              return VK_BLEND_FACTOR_DST_COLOR;
    case ERHIBlendFactor::OneMinusDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case ERHIBlendFactor::SrcAlpha:              return VK_BLEND_FACTOR_SRC_ALPHA;
    case ERHIBlendFactor::OneMinusSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case ERHIBlendFactor::DstAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
    case ERHIBlendFactor::OneMinusDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case ERHIBlendFactor::ConstantColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case ERHIBlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case ERHIBlendFactor::ConstantAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case ERHIBlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case ERHIBlendFactor::SrcAlphaSaturate:      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case ERHIBlendFactor::Src1Color:             return VK_BLEND_FACTOR_SRC1_COLOR;
    case ERHIBlendFactor::OneMinusSrc1Color:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case ERHIBlendFactor::Src1Alpha:             return VK_BLEND_FACTOR_SRC1_ALPHA;
    case ERHIBlendFactor::OneMinusSrc1Alpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:                                     return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp VulkanRHI::GetVkBlendOp(ERHIBlendOp Op)
{
    switch (Op)
    {
    case ERHIBlendOp::Add:             return VK_BLEND_OP_ADD;
    case ERHIBlendOp::Subtract:        return VK_BLEND_OP_SUBTRACT;
    case ERHIBlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case ERHIBlendOp::Min:             return VK_BLEND_OP_MIN;
    case ERHIBlendOp::Max:             return VK_BLEND_OP_MAX;
    default:                           return VK_BLEND_OP_ADD;
    }
}

VkFormat VulkanRHI::GetVkFormat(ERHITextureFormat Format)
{
    // TODO: use a switch
    if (Format == ERHITextureFormat::B8G8R8A8_SRGB) { return VK_FORMAT_B8G8R8A8_SRGB; }
    if (Format == ERHITextureFormat::R8G8B8A8_UNORM) { return VK_FORMAT_R8G8B8A8_UNORM; }
    if (Format == ERHITextureFormat::D32_SFLOAT) { return VK_FORMAT_D32_SFLOAT; }
    if (Format == ERHITextureFormat::D32_SFLOAT_S8_UINT) { return VK_FORMAT_D32_SFLOAT_S8_UINT; }

    return VK_FORMAT_UNDEFINED;
}

VkImageAspectFlags VulkanRHI::GetVkAspect(ERHITextureAspect Aspect)
{
    switch (Aspect)
    {
    case ERHITextureAspect::Color: return VK_IMAGE_ASPECT_COLOR_BIT;
    case ERHITextureAspect::Depth: return VK_IMAGE_ASPECT_DEPTH_BIT;
    case ERHITextureAspect::Stencil: return VK_IMAGE_ASPECT_STENCIL_BIT;
    case ERHITextureAspect::DepthStencil: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default: return VK_IMAGE_ASPECT_NONE;
    }
}

VkBufferUsageFlags VulkanRHI::GetVkBufferUsage(ERHIBufferUsage Usage)
{
    VkBufferUsageFlags Flags = 0;

    if (EnumHasAnyFlags(Usage, ERHIBufferUsage::Vertex)) Flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (EnumHasAnyFlags(Usage, ERHIBufferUsage::Index)) Flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (EnumHasAnyFlags(Usage, ERHIBufferUsage::TransferSrc)) Flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (EnumHasAnyFlags(Usage, ERHIBufferUsage::TransferDst)) Flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (EnumHasAnyFlags(Usage, ERHIBufferUsage::Storage)) Flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    return Flags;
}

VkFormat VulkanRHI::ToVkFormat(ERHIFormat Format)
{
    switch (Format)
    {
    case ERHIFormat::BGRA8_SRGB:        return VK_FORMAT_B8G8R8A8_SRGB;
    case ERHIFormat::RGBA8_UNorm:        return VK_FORMAT_R8G8B8A8_UNORM;
    case ERHIFormat::BGRA8_UNorm:        return VK_FORMAT_B8G8R8A8_UNORM;
    case ERHIFormat::RGBA16_Float:       return VK_FORMAT_R16G16B16A16_SFLOAT;
    case ERHIFormat::RGBA32_Float:       return VK_FORMAT_R32G32B32A32_SFLOAT;
    case ERHIFormat::D24S8:              return VK_FORMAT_D24_UNORM_S8_UINT;
    case ERHIFormat::D32_Float:          return VK_FORMAT_D32_SFLOAT;
    case ERHIFormat::D32_Float_S8X24:    return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case ERHIFormat::Float2:             return VK_FORMAT_R32G32_SFLOAT;
    case ERHIFormat::Float3:             return VK_FORMAT_R32G32B32_SFLOAT;
    case ERHIFormat::UNorm8x4:             return VK_FORMAT_R8G8B8A8_UNORM;
    default:                             return VK_FORMAT_UNDEFINED;
    }
}

VkPrimitiveTopology VulkanRHI::ToVkTopology(ERHIPrimitiveTopology Topology)
{
    switch (Topology)
    {
    case ERHIPrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case ERHIPrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case ERHIPrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case ERHIPrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case ERHIPrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case ERHIPrimitiveTopology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Fallback
    }
}

VkCullModeFlags VulkanRHI::ToVkCullMode(ERHICullMode Mode)
{
    switch (Mode)
    {
    case ERHICullMode::None:           return VK_CULL_MODE_NONE;
    case ERHICullMode::Front:          return VK_CULL_MODE_FRONT_BIT;
    case ERHICullMode::Back:           return VK_CULL_MODE_BACK_BIT;
    case ERHICullMode::FrontAndBack:   return VK_CULL_MODE_FRONT_AND_BACK;
    default:                           return VK_CULL_MODE_NONE;
    }
}

VkPolygonMode VulkanRHI::ToVkPolygonMode(ERHIPolygonMode Mode)
{
    switch (Mode)
    {
    case ERHIPolygonMode::Fill: return VK_POLYGON_MODE_FILL;
    case ERHIPolygonMode::Line: return VK_POLYGON_MODE_LINE;
    case ERHIPolygonMode::Point: return VK_POLYGON_MODE_POINT;
    default: return VK_POLYGON_MODE_FILL;
    }
}
VmaMemoryUsage VulkanRHI::ToVmaMemoryUsage(ERHIResourceMemory Memory)
{
    switch (Memory)
    {
    case ERHIResourceMemory::DeviceLocal: return VMA_MEMORY_USAGE_GPU_ONLY;
    case ERHIResourceMemory::HostVisible: return VMA_MEMORY_USAGE_CPU_TO_GPU;
    default: return VMA_MEMORY_USAGE_UNKNOWN;
    }
}

VkFilter VulkanRHI::ToVkFilter(ERHIFilterMode mode)
{
    switch (mode)
    {
    case ERHIFilterMode::Nearest: return VK_FILTER_NEAREST;
    case ERHIFilterMode::Linear:  return VK_FILTER_LINEAR;
    }
    return VK_FILTER_LINEAR;
}

VkSamplerAddressMode VulkanRHI::ToVkAddressMode(ERHIAddressMode mode)
{
    switch (mode)
    {
    case ERHIAddressMode::Repeat:         return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case ERHIAddressMode::MirroredRepeat: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case ERHIAddressMode::ClampToEdge:    return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case ERHIAddressMode::ClampToBorder:  return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    }
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

VkBorderColor VulkanRHI::ToVkBorderColor(ERHIBorderColor color)
{
    switch (color)
    {
    case ERHIBorderColor::FloatTransparentBlack: return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    case ERHIBorderColor::IntTransparentBlack:   return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
    case ERHIBorderColor::FloatOpaqueBlack:      return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    case ERHIBorderColor::IntOpaqueBlack:        return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    case ERHIBorderColor::FloatOpaqueWhite:      return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    case ERHIBorderColor::IntOpaqueWhite:        return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
    }
    return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
}

VkCompareOp VulkanRHI::ToVkCompareOp(ERHICompareOp op)
{
    switch (op)
    {
    case ERHICompareOp::Never:          return VK_COMPARE_OP_NEVER;
    case ERHICompareOp::Less:           return VK_COMPARE_OP_LESS;
    case ERHICompareOp::Equal:          return VK_COMPARE_OP_EQUAL;
    case ERHICompareOp::LessOrEqual:    return VK_COMPARE_OP_LESS_OR_EQUAL;
    case ERHICompareOp::Greater:        return VK_COMPARE_OP_GREATER;
    case ERHICompareOp::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
    case ERHICompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case ERHICompareOp::Always:         return VK_COMPARE_OP_ALWAYS;
    }
    return VK_COMPARE_OP_ALWAYS;
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
