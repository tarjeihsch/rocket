#pragma once
#include <cstdint>

#include "RHI.h"

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

enum class ERHITextureLayout : uint8_t
{
    Undefined,           // Initial undefined state
    General,             // Read/Write by shader (UAV)
    ColorAttachment,     // Render target
    DepthStencil,        // Depth/stencil usage
    DepthReadOnly,       // Shader read-only depth
    ShaderReadOnly,      // Sampled image
    TransferSrc,         // Copy from
    TransferDst,         // Copy to
    Present              // Swapchain presentation
};

enum class ERHIResourceAccess : uint16_t
{
    None = 0,

    // GPU Shader Access
    ShaderSampledRead         = 1 << 0,  // SRV: sampled image
    ShaderStorageRead         = 1 << 1,  // UAV read
    ShaderStorageWrite        = 1 << 2,  // UAV write
    InputAttachmentRead       = 1 << 3,  // Vulkan subpass input

    // Transfer Operations
    TransferRead              = 1 << 4,
    TransferWrite             = 1 << 5,

    // Attachment Writes
    ColorAttachmentWrite      = 1 << 6,
    DepthStencilAttachmentWrite = 1 << 7,

    // CPU Access (optional)
    HostRead                  = 1 << 8,
    HostWrite                 = 1 << 9,

    Presentation = 1 << 10,
};

enum class ERHIPipelineStage : uint16_t
{
    None              = 0,
    TopOfPipe         = 1 << 0,
    VertexShader      = 1 << 1,
    FragmentShader    = 1 << 2,
    ComputeShader     = 1 << 3,
    Transfer          = 1 << 4,
    ColorAttachment   = 1 << 5,
    DepthStencil      = 1 << 6,
    BottomOfPipe      = 1 << 7,
};

enum class ERHITextureFormat
{
    Invalid = 0,
    B8G8R8A8_SRGB,
    R8G8B8A8_UNORM,
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

inline ERHITextureUsage operator|(ERHITextureUsage Lhs, ERHITextureUsage Rhs) { return static_cast<ERHITextureUsage>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline ERHITextureUsage operator&(ERHITextureUsage Lhs, ERHITextureUsage Rhs) { return static_cast<ERHITextureUsage>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

struct FRHITextureDesc
{
    union
    {
        uint32_t Extent[2];
    } Tex2D;

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
        Desc.Tex2D.Extent[0] = SizeX;
        Desc.Tex2D.Extent[1] = SizeY;
        Desc.Format = Format;
        Desc.Lifetime = InLifetime;
        Desc.MipLevel = MipLevel;
        Desc.Usage = Usage;
        Desc.InitialLayout = InitialLayout;
        Desc.SampleCount = SampleCount;
        return Desc;
    }
};

struct FRHITexture
{
    virtual ~FRHITexture() = default;

    virtual const FRHITextureDesc& RHIGetTextureDesc() const = 0;
    virtual void* GetHandle() const = 0;

    ERHITextureLayout CurrentLayout = ERHITextureLayout::Undefined;
    ERHIResourceAccess CurrentAccess = ERHIResourceAccess::None;
    ERHIPipelineStage CurrentPipelineStage = ERHIPipelineStage::None;
    uint32_t CurrentQueueFamilyIndex = ~(0U);
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
    virtual void* GetHandle() const = 0;
    virtual uint32_t GetBindingIndex() const = 0;
};