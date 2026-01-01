#pragma once

#include "Common/RHI.h"

enum class RHIResourceLifetime : uint8_t;

enum class ERHITextureUsage : uint8_t
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

inline ERHITextureUsage operator|(ERHITextureUsage Lhs, ERHITextureUsage Rhs) { return static_cast<ERHITextureUsage>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline ERHITextureUsage operator&(ERHITextureUsage Lhs, ERHITextureUsage Rhs) { return static_cast<ERHITextureUsage>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

enum class RHITextureLayout : uint8_t
{
    None, // Initial undefined state
    General, // Read/Write by shader (UAV)
    ColorAttachment, // Render target
    DepthStencil, // Depth/stencil usage
    DepthReadOnly, // Shader read-only depth
    ShaderReadOnly, // Sampled image
    TransferSrc, // Copy from
    TransferDst, // Copy to
    Present // Swapchain presentation
};

enum class ERHITextureFormat
{
    Invalid = 0,
    B8G8R8A8_SRGB,
    R8G8B8A8_UNORM,
    R8G8B8A8_SRGB,
    D32_SFLOAT_S8_UINT,
    D32_SFLOAT,
};

enum class ERHITextureAspect
{
    None    = 0,
    Color   = 1 << 0,
    Depth   = 1 << 1,
    Stencil = 1 << 2,
    DepthStencil = Depth | Stencil
};

inline uint32_t GetTextureFormatPixelByteSize(ERHITextureFormat Format)
{
    switch (Format)
    {
	case ERHITextureFormat::B8G8R8A8_SRGB:
	case ERHITextureFormat::R8G8B8A8_SRGB:
    case ERHITextureFormat::R8G8B8A8_UNORM:
        return 4;
    case ERHITextureFormat::D32_SFLOAT:
        return 4;
    case ERHITextureFormat::D32_SFLOAT_S8_UINT:
        return 8;
    default:
        return 0;
    }
}

struct RHITextureDesc
{
    uint32_t extent[2];
    uint32_t Offset[2];

    RHIResourceLifetime Lifetime;
    ERHITextureFormat Format;
    ERHITextureUsage Usage;
    RHITextureLayout initial_layout;
    uint32_t MipLevel;
    uint32_t LayerCount;
    uint32_t SampleCount;

    static RHITextureDesc Create2D(uint32_t SizeX, uint32_t SizeY, RHIResourceLifetime InLifetime, ERHITextureFormat Format, ERHITextureUsage Usage, RHITextureLayout InitialLayout, uint32_t MipLevel, uint32_t LayerCount, uint32_t SampleCount)
    {
        RHITextureDesc Desc = {};
        Desc.extent[0] = SizeX;
        Desc.extent[1] = SizeY;
        Desc.Format = Format;
        Desc.Lifetime = InLifetime;
        Desc.MipLevel = MipLevel;
        Desc.Usage = Usage;
        Desc.initial_layout = InitialLayout;
        Desc.SampleCount = SampleCount;
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

    static RHITextureAccessState Present()
    {
        RHITextureAccessState state = {};
        state.texture_layout = RHITextureLayout::Present;
        state.pipeline_stage = RHIPipelineStage::BottomOfPipe;
        state.resource_access = RHIResourceAccess::None;
        state.queue_family_index = UINT32_MAX;
        return state;
    }
};

struct RHITexture
{
    RHITexture(const RHITextureDesc& in_desc, const RHITextureAccessState& in_texture_access_state)
        : texture_desc(in_desc),
        texture_access_state(in_texture_access_state)
    {
    }

    virtual ~RHITexture() = default;

    virtual void* GetHandle() const = 0;

    __inline const RHITextureDesc& GetTextureDesc() const
    {
        return texture_desc;
    }

    __inline const RHITextureAccessState& GetTextureAccessState() const
    {
        return texture_access_state;
    }

    __inline void SetTextureAccessState(const RHITextureAccessState& in_texture_access_state)
    {
        texture_access_state = in_texture_access_state;
    }

private:
    RHITextureDesc texture_desc;
    RHITextureAccessState texture_access_state;
};

struct RHITextureViewDesc
{
    ERHITextureFormat Format = ERHITextureFormat::Invalid;
    ERHITextureAspect aspect = ERHITextureAspect::None;
    RHIResourceLifetime Lifetime;

    static RHITextureViewDesc Create2D(ERHITextureFormat Format, ERHITextureAspect Aspect, RHIResourceLifetime Lifetime)
    {
        RHITextureViewDesc Desc = {};
        Desc.Format = Format;
        Desc.aspect = Aspect;
        Desc.Lifetime = Lifetime;
        return Desc;
    }
};

struct RHITextureView
{
    virtual ~RHITextureView() = default;

    virtual const RHITextureViewDesc& get_desc() const = 0;
    virtual RHITexture* get_target_texture() const = 0;
    virtual void* get_handle() const = 0;
    virtual uint32_t GetBindingIndex() const = 0;
};
