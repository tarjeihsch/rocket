#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>

#include "VulkanLoader.h"

#include "Common/RHICommon.h"
#include "Common/RHIPipeline.h"
#include "Common/RHITextureSampler.h"
#include "Common/RHITexture.h"
#include "Common/RHIBuffer.h"
#include "Vulkan/VulkanRHIMinimal.h"

#include <assert.h>

#include "Common/RHISemaphore.h"

#ifdef RK_PLATFORM_WINDOWS
    #include <windows.h>
    #define VK_DLSYM(lib, name) GetProcAddress((HMODULE)(lib), name)
#endif

#ifdef RK_PLATFORM_LINUX
    #include <dlfcn.h>
    #define VK_DLSYM(lib, name) dlsym(lib, name)
#endif
#include <Assertion.h>

namespace VulkanRHI
{
    void* Library = nullptr;

    #define DEFINE_PFN(name) PFN_##name name = nullptr;
    VULKAN_ALL_FUNCTIONS(DEFINE_PFN)
    #undef DEFINE_PFN
};

template <typename TEnum>
inline bool EnumHasAnyFlags(TEnum Value, TEnum Flags)
{
    using T = std::underlying_type_t<TEnum>;
    return (static_cast<T>(Value) & static_cast<T>(Flags)) != 0;
}

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

VkAccessFlags2 VulkanRHI::GetVkAccess(RHIResourceAccess access)
{
    VkAccessFlags2 flags = 0;
    if (EnumHasAnyFlags(access, RHIResourceAccess::ShaderSampledRead))      flags |= VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::ShaderStorageRead))      flags |= VK_ACCESS_2_SHADER_STORAGE_READ_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::ShaderStorageWrite))     flags |= VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::InputAttachmentRead))    flags |= VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::TransferRead))           flags |= VK_ACCESS_2_TRANSFER_READ_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::TransferWrite))          flags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::ColorAttachmentWrite))   flags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::DepthStencilAttachmentWrite)) flags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::HostRead))               flags |= VK_ACCESS_2_HOST_READ_BIT;
    if (EnumHasAnyFlags(access, RHIResourceAccess::HostWrite))              flags |= VK_ACCESS_2_HOST_WRITE_BIT;
    return flags;
}

VkImageLayout VulkanRHI::GetVulkanLayout(RHITextureLayout layout)
{
    switch (layout)
    {
    case RHITextureLayout::None:         return VK_IMAGE_LAYOUT_UNDEFINED;
    case RHITextureLayout::General:           return VK_IMAGE_LAYOUT_GENERAL;
    case RHITextureLayout::ColorAttachment:   return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case RHITextureLayout::DepthStencil:      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case RHITextureLayout::DepthReadOnly:     return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    case RHITextureLayout::ShaderReadOnly:    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    case RHITextureLayout::TransferSrc:       return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case RHITextureLayout::TransferDst:       return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case RHITextureLayout::Present:           return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    default:                                   return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

VkPipelineStageFlags2 VulkanRHI::GetVkPipelineStage(RHIPipelineStage stage)
{
    VkPipelineStageFlags2 flags = 0;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::TopOfPipe))       flags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::VertexShader))    flags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::FragmentShader))  flags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::ComputeShader))   flags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::Transfer))        flags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::ColorAttachment)) flags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::DepthStencil))    flags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::BottomOfPipe))    flags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
    if (EnumHasAnyFlags(stage, RHIPipelineStage::AllCommands))    flags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    return flags;
}

VkImageUsageFlags VulkanRHI::GetVulkanUsage(RHITextureUsage usage)
{
    VkImageUsageFlags flags = 0;

    if (EnumHasAnyFlags(usage, RHITextureUsage::Present))  flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Used indirectly
    if (EnumHasAnyFlags(usage, RHITextureUsage::Color))    flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (EnumHasAnyFlags(usage, RHITextureUsage::Depth))    flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (EnumHasAnyFlags(usage, RHITextureUsage::Stencil))  flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    if (EnumHasAnyFlags(usage, RHITextureUsage::CopySrc))  flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    if (EnumHasAnyFlags(usage, RHITextureUsage::CopyDst))  flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (EnumHasAnyFlags(usage, RHITextureUsage::SRV))      flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    if (EnumHasAnyFlags(usage, RHITextureUsage::UAV))      flags |= VK_IMAGE_USAGE_STORAGE_BIT;

    return flags;
}

VkBlendFactor VulkanRHI::GetVkBlendFactor(RHIBlendFactor Factor)
{
    switch (Factor)
    {
    case RHIBlendFactor::Zero:                  return VK_BLEND_FACTOR_ZERO;
    case RHIBlendFactor::One:                   return VK_BLEND_FACTOR_ONE;
    case RHIBlendFactor::SrcColor:              return VK_BLEND_FACTOR_SRC_COLOR;
    case RHIBlendFactor::OneMinusSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case RHIBlendFactor::DstColor:              return VK_BLEND_FACTOR_DST_COLOR;
    case RHIBlendFactor::OneMinusDstColor:      return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case RHIBlendFactor::SrcAlpha:              return VK_BLEND_FACTOR_SRC_ALPHA;
    case RHIBlendFactor::OneMinusSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case RHIBlendFactor::DstAlpha:              return VK_BLEND_FACTOR_DST_ALPHA;
    case RHIBlendFactor::OneMinusDstAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case RHIBlendFactor::ConstantColor:         return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case RHIBlendFactor::OneMinusConstantColor: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
    case RHIBlendFactor::ConstantAlpha:         return VK_BLEND_FACTOR_CONSTANT_ALPHA;
    case RHIBlendFactor::OneMinusConstantAlpha: return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
    case RHIBlendFactor::SrcAlphaSaturate:      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case RHIBlendFactor::Src1Color:             return VK_BLEND_FACTOR_SRC1_COLOR;
    case RHIBlendFactor::OneMinusSrc1Color:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case RHIBlendFactor::Src1Alpha:             return VK_BLEND_FACTOR_SRC1_ALPHA;
    case RHIBlendFactor::OneMinusSrc1Alpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    default:                                     return VK_BLEND_FACTOR_ONE;
    }
}

VkBlendOp VulkanRHI::GetVkBlendOp(RHIBlendOp Op)
{
    switch (Op)
    {
    case RHIBlendOp::Add:             return VK_BLEND_OP_ADD;
    case RHIBlendOp::Subtract:        return VK_BLEND_OP_SUBTRACT;
    case RHIBlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
    case RHIBlendOp::Min:             return VK_BLEND_OP_MIN;
    case RHIBlendOp::Max:             return VK_BLEND_OP_MAX;
    default:                           return VK_BLEND_OP_ADD;
    }
}

VkFormat VulkanRHI::GetVkFormat(RHITextureFormat Format)
{
    // TODO: use a switch
    if (Format == RHITextureFormat::B8G8R8A8_SRGB) { return VK_FORMAT_B8G8R8A8_SRGB; }
    if (Format == RHITextureFormat::R8G8B8A8_UNORM) { return VK_FORMAT_R8G8B8A8_UNORM; }
    if (Format == RHITextureFormat::D32_SFLOAT) { return VK_FORMAT_D32_SFLOAT; }
    if (Format == RHITextureFormat::D32_SFLOAT_S8_UINT) { return VK_FORMAT_D32_SFLOAT_S8_UINT; }

    return VK_FORMAT_UNDEFINED;
}

VkImageAspectFlags VulkanRHI::GetVkAspect(RHITextureAspect Aspect)
{
    switch (Aspect)
    {
    case RHITextureAspect::Color: return VK_IMAGE_ASPECT_COLOR_BIT;
    case RHITextureAspect::Depth: return VK_IMAGE_ASPECT_DEPTH_BIT;
    case RHITextureAspect::Stencil: return VK_IMAGE_ASPECT_STENCIL_BIT;
    case RHITextureAspect::DepthStencil: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    default: return VK_IMAGE_ASPECT_NONE;
    }
}

VkBufferUsageFlags VulkanRHI::GetVkBufferUsage(RHIBufferUsage Usage)
{
    VkBufferUsageFlags Flags = 0;

    if (EnumHasAnyFlags(Usage, RHIBufferUsage::Vertex)) Flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if (EnumHasAnyFlags(Usage, RHIBufferUsage::Index)) Flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    if (EnumHasAnyFlags(Usage, RHIBufferUsage::TransferSrc)) Flags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (EnumHasAnyFlags(Usage, RHIBufferUsage::TransferDst)) Flags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    if (EnumHasAnyFlags(Usage, RHIBufferUsage::Storage)) Flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    return Flags;
}

VkFormat VulkanRHI::ToVkFormat(RHIFormat Format)
{
    switch (Format)
    {
    case RHIFormat::BGRA8_SRGB:        return VK_FORMAT_B8G8R8A8_SRGB;
    case RHIFormat::RGBA8_UNorm:        return VK_FORMAT_R8G8B8A8_UNORM;
    case RHIFormat::BGRA8_UNorm:        return VK_FORMAT_B8G8R8A8_UNORM;
    case RHIFormat::RGBA16_Float:       return VK_FORMAT_R16G16B16A16_SFLOAT;
    case RHIFormat::RGBA32_Float:       return VK_FORMAT_R32G32B32A32_SFLOAT;
    case RHIFormat::D24S8:              return VK_FORMAT_D24_UNORM_S8_UINT;
    case RHIFormat::D32_Float:          return VK_FORMAT_D32_SFLOAT;
    case RHIFormat::D32_Float_S8X24:    return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case RHIFormat::Float2:             return VK_FORMAT_R32G32_SFLOAT;
    case RHIFormat::Float3:             return VK_FORMAT_R32G32B32_SFLOAT;
    case RHIFormat::UNorm8x4:             return VK_FORMAT_R8G8B8A8_UNORM;
    default:                             return VK_FORMAT_UNDEFINED;
    }
}

VkImageAspectFlags VulkanRHI::GetVkAspectFromFormat(RHITextureFormat format)
{
    switch (format)
    {
        case RHITextureFormat::B8G8R8A8_SRGB:
        case RHITextureFormat::R8G8B8A8_UNORM:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        case RHITextureFormat::D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case RHITextureFormat::D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_NONE;
    }
}

VkPrimitiveTopology VulkanRHI::ToVkTopology(RHIPrimitiveTopology Topology)
{
    switch (Topology)
    {
    case RHIPrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case RHIPrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case RHIPrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case RHIPrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case RHIPrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case RHIPrimitiveTopology::PatchList: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
    default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Fallback
    }
}

VkCullModeFlags VulkanRHI::ToVkCullMode(RHICullMode Mode)
{
    switch (Mode)
    {
    case RHICullMode::None:           return VK_CULL_MODE_NONE;
    case RHICullMode::Front:          return VK_CULL_MODE_FRONT_BIT;
    case RHICullMode::Back:           return VK_CULL_MODE_BACK_BIT;
    case RHICullMode::FrontAndBack:   return VK_CULL_MODE_FRONT_AND_BACK;
    default:                           return VK_CULL_MODE_NONE;
    }
}

VkPolygonMode VulkanRHI::ToVkPolygonMode(RHIPolygonMode Mode)
{
    switch (Mode)
    {
    case RHIPolygonMode::Fill: return VK_POLYGON_MODE_FILL;
    case RHIPolygonMode::Line: return VK_POLYGON_MODE_LINE;
    case RHIPolygonMode::Point: return VK_POLYGON_MODE_POINT;
    default: return VK_POLYGON_MODE_FILL;
    }
}
VmaMemoryUsage VulkanRHI::ToVmaMemoryUsage(RHIResourceMemory Memory)
{
    switch (Memory)
    {
    case RHIResourceMemory::DeviceLocal: return VMA_MEMORY_USAGE_GPU_ONLY;
    case RHIResourceMemory::HostVisible: return VMA_MEMORY_USAGE_CPU_TO_GPU;
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

VkPipelineStageFlags2 VulkanRHI::ToVkStage2(RHISyncStage Stage)
{
    switch (Stage)
    {
        case RHISyncStage::TopOfPipe:                 return VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
        case RHISyncStage::ColorAttachmentOutput:     return VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        case RHISyncStage::AllCommands:               return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        default:                                            return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    }
}


void LoadVulkanLibrary()
{
#if RK_PLATFORM_WINDOWS
    VulkanRHI::Library = LoadLibraryA("vulkan-1.dll");
    ensure(VulkanRHI::Library != nullptr, "Failed to load Vulkan library.");
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
