#pragma once

#include "Common/RHI.h"
#include "Common/RHICommon.h"

enum class RHIResourceLifetime : uint8_t;

enum class RHITextureUsage : uint8_t
{
    None = 0,
    Present = 1 << 0, // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT (indirectly used for presentation via swapchain)
    Color = 1 << 1, // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    Depth = 1 << 2, // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    Stencil = 1 << 3, // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT (used with stencil aspects)
    CopySrc = 1 << 4, // VK_IMAGE_USAGE_TRANSFER_SRC_BIT
    CopyDst = 1 << 5, // VK_IMAGE_USAGE_TRANSFER_DST_BIT
    SRV = 1 << 6, // VK_IMAGE_USAGE_SAMPLED_BIT
    UAV = 1 << 7  // VK_IMAGE_USAGE_STORAGE_BIT
};

inline RHITextureUsage operator|(RHITextureUsage Lhs, RHITextureUsage Rhs) { return static_cast<RHITextureUsage>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline RHITextureUsage operator&(RHITextureUsage Lhs, RHITextureUsage Rhs) { return static_cast<RHITextureUsage>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

inline uint32_t GetTextureFormatPixelByteSize(RHITextureFormat Format)
{
    switch (Format)
    {
	case RHITextureFormat::B8G8R8A8_SRGB:
	case RHITextureFormat::R8G8B8A8_SRGB:
    case RHITextureFormat::R8G8B8A8_UNORM:
        return 4;
    case RHITextureFormat::D32_SFLOAT:
        return 4;
    case RHITextureFormat::D32_SFLOAT_S8_UINT:
        return 8;
    default:
        return 0;
    }
}

struct RHITextureDesc
{
	const char* DebugName = nullptr;

    uint32_t extent[2];
    uint32_t offset[2];

    RHIResourceLifetime lifetime;
    RHITextureFormat format;
    RHITextureUsage usage;
    RHITextureLayout initial_layout;
    uint32_t NumMips;
    uint32_t LayerCount;
    uint32_t SampleCount;

    static RHITextureDesc Create2D(uint32_t SizeX, uint32_t SizeY, RHIResourceLifetime InLifetime, RHITextureFormat Format, RHITextureUsage Usage, RHITextureLayout InitialLayout, uint32_t MipLevel, uint32_t LayerCount, uint32_t SampleCount)
    {
        RHITextureDesc Desc = {};
        Desc.extent[0] = SizeX;
        Desc.extent[1] = SizeY;
        Desc.format = Format;
        Desc.lifetime = InLifetime;
        Desc.NumMips = MipLevel;
        Desc.usage = Usage;
        Desc.initial_layout = InitialLayout;
        Desc.SampleCount = SampleCount;
        return Desc;
    }

    static RHITextureDesc CreateColorRT(uint32_t SizeX, uint32_t SizeY, RHITextureFormat Format)
    {
        RHITextureDesc Desc = {};
        Desc.extent[0] = SizeX;
        Desc.extent[1] = SizeY;
        Desc.format = Format;
        Desc.lifetime = RHIResourceLifetime::Transient;
        Desc.usage = RHITextureUsage::Color | RHITextureUsage::SRV | RHITextureUsage::CopySrc;
        Desc.initial_layout = RHITextureLayout::ColorAttachment;
        Desc.NumMips = 1;
        Desc.LayerCount = 1;
        Desc.SampleCount = 1;
        return Desc;
    }

    static RHITextureDesc CreateDepthRT(uint32_t SizeX, uint32_t SizeY, RHITextureFormat Format)
    {
        RHITextureDesc Desc = {};
        Desc.extent[0] = SizeX;
        Desc.extent[1] = SizeY;
        Desc.format = Format;
        Desc.lifetime = RHIResourceLifetime::Transient;
        Desc.usage = RHITextureUsage::Depth | RHITextureUsage::Stencil;
        Desc.initial_layout = RHITextureLayout::DepthStencil;
        Desc.NumMips = 0;
        Desc.LayerCount = 1;
        Desc.SampleCount = 1;
        return Desc;
    }
};

struct RHITextureAccessState
{
    RHITextureLayout texture_layout;
    RHIResourceAccess resource_access;
    RHIPipelineStage pipeline_stage;
    uint32_t queue_family_index;

    static RHITextureAccessState Default()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::None;
        state.pipeline_stage = RHIPipelineStage::None;
        state.resource_access = RHIResourceAccess::None;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHITextureAccessState ShaderRead()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::ShaderReadOnly;
        state.pipeline_stage = RHIPipelineStage::VertexShader | RHIPipelineStage::FragmentShader | RHIPipelineStage::ComputeShader;
        state.resource_access = RHIResourceAccess::ShaderSampledRead;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHITextureAccessState UnorderedAccess()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::General;
        state.pipeline_stage = RHIPipelineStage::VertexShader | RHIPipelineStage::FragmentShader | RHIPipelineStage::ComputeShader;
        state.resource_access = RHIResourceAccess::ShaderStorageRead | RHIResourceAccess::ShaderStorageWrite;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHITextureAccessState CopySrc()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::TransferSrc;
        state.pipeline_stage = RHIPipelineStage::Transfer;
        state.resource_access = RHIResourceAccess::TransferRead;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHITextureAccessState CopyDst()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::TransferDst;
        state.pipeline_stage = RHIPipelineStage::Transfer;
        state.resource_access = RHIResourceAccess::TransferWrite;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHITextureAccessState Present()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::Present;
        state.pipeline_stage = RHIPipelineStage::BottomOfPipe;
        state.resource_access = RHIResourceAccess::None;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHITextureAccessState ColorAttachment()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::ColorAttachment;
        state.pipeline_stage = RHIPipelineStage::ColorAttachment;
        state.resource_access = RHIResourceAccess::ColorAttachmentWrite;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

    static RHITextureAccessState DepthStencilAttachment()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::DepthStencil;
        state.pipeline_stage = RHIPipelineStage::DepthStencil;
        state.resource_access = RHIResourceAccess::DepthStencilAttachmentWrite;
        state.queue_family_index = UINT32_MAX;
        return state;
    }

	bool operator==(const RHITextureAccessState& other) const noexcept 
    {
        if (this->texture_layout == other.texture_layout
            && this->pipeline_stage == other.pipeline_stage
            && this->resource_access == other.resource_access
            && this->queue_family_index == other.queue_family_index)
        {
            return true;
        }
        return false;
    }

    bool operator!=(const RHITextureAccessState& other) const noexcept
    {
        return !(*this == other);
    }
};

struct RHITexture
{
    RHITexture(const RHITextureDesc& in_desc)
        : texture_desc(in_desc)
    {
    }
    virtual ~RHITexture() = default;

    virtual void* GetNativeTexture() const = 0;
    virtual RHITextureView* GetDefaultView() = 0;

    __inline const RHITextureDesc& GetTextureDesc() const
    {
        return texture_desc;
    }

private:
    RHITextureDesc texture_desc;
};