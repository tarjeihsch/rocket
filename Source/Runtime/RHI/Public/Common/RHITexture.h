#pragma once

#include "Common/RHI.h"

enum class ERHIResourceLifetime : uint8_t;

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

enum class ERHITextureLayout : uint8_t
{
    Undefined, // Initial undefined state
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

struct FRHITextureDesc
{
    uint32_t Extent[2];
    uint32_t Offset[2];

    ERHIResourceLifetime Lifetime;
    ERHITextureFormat Format;
    ERHITextureUsage Usage;
    ERHITextureLayout InitialLayout;
    uint32_t MipLevel;
    uint32_t LayerCount;
    uint32_t SampleCount;

    static FRHITextureDesc Create2D(uint32_t SizeX, uint32_t SizeY, ERHIResourceLifetime InLifetime, ERHITextureFormat Format, ERHITextureUsage Usage, ERHITextureLayout InitialLayout, uint32_t MipLevel, uint32_t LayerCount, uint32_t SampleCount)
    {
        FRHITextureDesc Desc = {};
        Desc.Extent[0] = SizeX;
        Desc.Extent[1] = SizeY;
        Desc.Format = Format;
        Desc.Lifetime = InLifetime;
        Desc.MipLevel = MipLevel;
        Desc.Usage = Usage;
        Desc.InitialLayout = InitialLayout;
        Desc.SampleCount = SampleCount;
        return Desc;
    }
};

struct RHITexture
{
    virtual ~RHITexture() = default;

    virtual const FRHITextureDesc& RHIGetTextureDesc() const = 0;
    virtual void* GetHandle() const = 0;
    
    virtual void gpu_barrier(RHICommandList& cmd_list, ERHITextureLayout new_layout, ERHIResourceAccess new_access, ERHIPipelineStage new_pipeline_stage) = 0;

    ERHITextureLayout current_layout = ERHITextureLayout::Undefined;
    ERHIResourceAccess current_access = ERHIResourceAccess::None;
    ERHIPipelineStage current_pipeline_stage = ERHIPipelineStage::None;
    uint32_t CurrentQueueFamilyIndex = ~(0U);

    uint8_t m_is_back_buffer : 1;
};

struct FRHITextureViewDesc
{
    ERHITextureFormat Format = ERHITextureFormat::Invalid;
    ERHITextureAspect Aspect = ERHITextureAspect::None;
    ERHIResourceLifetime Lifetime;

    static FRHITextureViewDesc Create2D(ERHITextureFormat Format, ERHITextureAspect Aspect, ERHIResourceLifetime Lifetime)
    {
        FRHITextureViewDesc Desc = {};
        Desc.Format = Format;
        Desc.Aspect = Aspect;
        Desc.Lifetime = Lifetime;
        return Desc;
    }
};

struct FRHITextureView
{
    virtual ~FRHITextureView() = default;

    virtual const FRHITextureViewDesc& RHIGetTextureViewDesc() const = 0;
    virtual RHITexture* GetTexture() const = 0;
    virtual void* GetHandle() const = 0;
    virtual uint32_t GetBindingIndex() const = 0;
};
