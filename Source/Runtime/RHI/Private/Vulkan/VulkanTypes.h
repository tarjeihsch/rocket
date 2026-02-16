#pragma once

#include "Common/RHITypes.h"
#include "VulkanLoader.h"

namespace RHI
{
    static VkFormat GetNativeFormat(GPUFormat format)
    {
        switch (format)
        {
            case GPUFormat::Unknown:         return VK_FORMAT_UNDEFINED;

            case GPUFormat::RGBA8_UNORM:     return VK_FORMAT_R8G8B8A8_UNORM;
            case GPUFormat::RGBA8_SRGB:      return VK_FORMAT_R8G8B8A8_SRGB;
            case GPUFormat::BGRA8_UNORM:     return VK_FORMAT_B8G8R8A8_UNORM;
            case GPUFormat::BGRA8_SRGB:      return VK_FORMAT_B8G8R8A8_SRGB;

            case GPUFormat::R16_FLOAT:       return VK_FORMAT_R16_SFLOAT;
            case GPUFormat::RG16_FLOAT:      return VK_FORMAT_R16G16_SFLOAT;
            case GPUFormat::RGBA16_FLOAT:    return VK_FORMAT_R16G16B16A16_SFLOAT;

            case GPUFormat::R32_FLOAT:       return VK_FORMAT_R32_SFLOAT;
            case GPUFormat::RG32_FLOAT:      return VK_FORMAT_R32G32_SFLOAT;
            case GPUFormat::RGB32_FLOAT:     return VK_FORMAT_R32G32B32_SFLOAT;
            case GPUFormat::RGBA32_FLOAT:    return VK_FORMAT_R32G32B32A32_SFLOAT;

            case GPUFormat::RGBA32_UINT:     return VK_FORMAT_R32G32B32A32_UINT;
            case GPUFormat::RGBA32_SINT:     return VK_FORMAT_R32G32B32A32_SINT;
            case GPUFormat::RGB32_UINT:      return VK_FORMAT_R32G32B32_UINT;
            case GPUFormat::RGB32_SINT:      return VK_FORMAT_R32G32B32_SINT;
            case GPUFormat::R32_UINT:        return VK_FORMAT_R32_UINT;

            case GPUFormat::D16_UNORM:       return VK_FORMAT_D16_UNORM;
            case GPUFormat::D32_FLOAT:       return VK_FORMAT_D32_SFLOAT;
            case GPUFormat::D24_UNORM_S8:    return VK_FORMAT_D24_UNORM_S8_UINT;
            case GPUFormat::D32_FLOAT_S8:    return VK_FORMAT_D32_SFLOAT_S8_UINT;
        }

        return VK_FORMAT_UNDEFINED;
    }

    static void GetBarrierInfo(GPUAccessType access_type, VkPipelineStageFlags2& stage, VkAccessFlags2& access)
    {
        switch (access_type)
        {
        case GPUAccessType::Undefined:
            stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            access = VK_ACCESS_2_NONE;
            break;

        case GPUAccessType::General:
            stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            break;

        case GPUAccessType::VertexShaderRead:
            stage = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
            access = VK_ACCESS_2_SHADER_READ_BIT;
            break;

        case GPUAccessType::FragmentShaderRead:
            stage = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
            access = VK_ACCESS_2_SHADER_READ_BIT;
            break;

        case GPUAccessType::ComputeShaderRead:
            stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            access = VK_ACCESS_2_SHADER_READ_BIT;
            break;

        case GPUAccessType::ComputeShaderWrite:
            stage = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            access = VK_ACCESS_2_SHADER_WRITE_BIT;
            break;

        case GPUAccessType::TransferSrc:
            stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            access = VK_ACCESS_2_TRANSFER_READ_BIT;
            break;

        case GPUAccessType::TransferDst:
            stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;

        default:
            stage = VK_PIPELINE_STAGE_2_NONE;
            access = VK_ACCESS_2_NONE;
            break;
        }
    }

    static void GetBarrierInfo(GPUTextureLayout layout, VkImageLayout& vk_layout, VkPipelineStageFlags2& stage, VkAccessFlags2& access)
    {
        switch (layout)
        {
        case GPUTextureLayout::Undefined:
            vk_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            stage = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
            access = VK_ACCESS_2_NONE;
            break;
        case GPUTextureLayout::General:
            vk_layout = VK_IMAGE_LAYOUT_GENERAL;
            stage = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            access = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT;
            break;
        case GPUTextureLayout::ColorAttachment:
            vk_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case GPUTextureLayout::DepthAttachment:
            vk_layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
            access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case GPUTextureLayout::ShaderReadOnly:
            vk_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            stage = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            access = VK_ACCESS_2_SHADER_READ_BIT;
            break;
        case GPUTextureLayout::TransferSrc:
            vk_layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            access = VK_ACCESS_2_TRANSFER_READ_BIT;
            break;
        case GPUTextureLayout::TransferDst:
            vk_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            stage = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
            access = VK_ACCESS_2_TRANSFER_WRITE_BIT;
            break;
        case GPUTextureLayout::Present:
            vk_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            stage = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
            access = VK_ACCESS_2_NONE;
            break;
        }
    }

    static VkShaderStageFlags GetNativeShaderStageFlags(GPUShaderStageFlags flags)
    {
        VkShaderStageFlags result = 0;
        if (flags & GPUShaderStageFlag::Vertex)   result |= VK_SHADER_STAGE_VERTEX_BIT;
        if (flags & GPUShaderStageFlag::Fragment) result |= VK_SHADER_STAGE_FRAGMENT_BIT;
        if (flags & GPUShaderStageFlag::Compute)  result |= VK_SHADER_STAGE_COMPUTE_BIT;
        return result;
    }

    static VkCullModeFlags GetNativeCullMode(GPUCullMode mode)
    {
        switch (mode)
        {
            case GPUCullMode::None:  return VK_CULL_MODE_NONE;
            case GPUCullMode::Front: return VK_CULL_MODE_FRONT_BIT;
            case GPUCullMode::Back:  return VK_CULL_MODE_BACK_BIT;
            default: return VK_CULL_MODE_NONE;
        }
    }

    static VkFrontFace GetNativeFrontFace(GPUFrontFace face)
    {
        switch (face)
        {
            case GPUFrontFace::Clockwise: return VK_FRONT_FACE_CLOCKWISE;
            case GPUFrontFace::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
            default: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        }
    }

    static VkPolygonMode GetNativePolygonMode(GPUPolygonMode mode)
    {
        switch (mode)
        {
            case GPUPolygonMode::Fill:  return VK_POLYGON_MODE_FILL;
            case GPUPolygonMode::Line:  return VK_POLYGON_MODE_LINE;
            case GPUPolygonMode::Point: return VK_POLYGON_MODE_POINT;
            default: return VK_POLYGON_MODE_FILL;
        }
    }

    static VkPrimitiveTopology GetNativeTopology(GPUPrimitiveTopology topology)
    {
        switch (topology)
        {
            case GPUPrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case GPUPrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            case GPUPrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            case GPUPrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
            case GPUPrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
            default: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    static VkCompareOp GetNativeCompareOp(GPUCompareOp op)
    {
        switch (op)
        {
            case GPUCompareOp::Never: return VK_COMPARE_OP_NEVER;
            case GPUCompareOp::Less: return VK_COMPARE_OP_LESS;
            case GPUCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
            case GPUCompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
            case GPUCompareOp::Greater: return VK_COMPARE_OP_GREATER;
            case GPUCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
            case GPUCompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case GPUCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
            default: return VK_COMPARE_OP_LESS;
        }
    }

    static VkBlendFactor GetNativeBlendFactor(GPUBlendFactor factor)
    {
        switch (factor)
        {
            case GPUBlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
            case GPUBlendFactor::One: return VK_BLEND_FACTOR_ONE;
            case GPUBlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
            case GPUBlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
            case GPUBlendFactor::DstColor: return VK_BLEND_FACTOR_DST_COLOR;
            case GPUBlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
            case GPUBlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
            case GPUBlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            case GPUBlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
            case GPUBlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
            default: return VK_BLEND_FACTOR_ONE;
        }
    }

    static VkBlendOp GetNativeBlendOp(GPUBlendOp op)
    {
        switch (op)
        {
            case GPUBlendOp::Add: return VK_BLEND_OP_ADD;
            case GPUBlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
            case GPUBlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
            case GPUBlendOp::Min: return VK_BLEND_OP_MIN;
            case GPUBlendOp::Max: return VK_BLEND_OP_MAX;
            default: return VK_BLEND_OP_ADD;
        }
    }

    static VkPresentModeKHR GetNativePresentMode(GPUPresentMode mode)
    {
        switch (mode)
        {
            case GPUPresentMode::Immediate: return VK_PRESENT_MODE_IMMEDIATE_KHR;
            case GPUPresentMode::FIFO: return VK_PRESENT_MODE_FIFO_KHR;
            case GPUPresentMode::FIFOAdaptive: return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
            case GPUPresentMode::Mailbox: return VK_PRESENT_MODE_MAILBOX_KHR;
            default: return VK_PRESENT_MODE_FIFO_KHR;
        }
    }
}
