#pragma once

#include "Common/RHI.h"

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

enum class RHITextureFormat
{
    Invalid = 0,
    B8G8R8A8_SRGB,
    R8G8B8A8_UNORM,
    R8G8B8A8_SRGB,
    D32_SFLOAT_S8_UINT,
    D32_SFLOAT,
};

enum class RHITextureAspect
{
    None    = 0,
    Color   = 1 << 0,
    Depth   = 1 << 1,
    Stencil = 1 << 2,
    DepthStencil = Depth | Stencil
};

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
    RHITexture(const RHITextureDesc& in_desc, const RHITextureAccessState& in_texture_access_state)
        : texture_desc(in_desc),
        texture_access_state(in_texture_access_state)
    {
    }
    virtual ~RHITexture() = default;

    virtual void* GetNativeTexture() const = 0;
    virtual RHITextureView* GetDefaultView() = 0;

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
    RHITextureFormat Format = RHITextureFormat::Invalid;
    RHITextureAspect aspect = RHITextureAspect::None;
    RHIResourceLifetime Lifetime;
	uint32_t BaseMipLevel = 0;
	uint32_t NumMips = 1;
	uint32_t BaseArrayLayer = 0;
	uint32_t NumArrayLayers = 1;

    static RHITextureViewDesc Create2D(RHITextureFormat Format, RHITextureAspect Aspect, RHIResourceLifetime Lifetime)
    {
        RHITextureViewDesc Desc = {};
        Desc.Format = Format;
        Desc.aspect = Aspect;
        Desc.Lifetime = Lifetime;
		Desc.BaseMipLevel = 0;
		Desc.NumMips = 1;
		Desc.BaseArrayLayer = 0;
		Desc.NumArrayLayers = 1;
        return Desc;
    }

	static RHITextureViewDesc CreateFromTexture(const RHITextureDesc& TexDesc)
	{
		RHITextureViewDesc Desc = {};
		Desc.Format = TexDesc.format;
		Desc.aspect = RHITextureAspect::Color;
		Desc.BaseMipLevel = 0;
		Desc.NumMips = TexDesc.NumMips;
		Desc.BaseArrayLayer = 0;
		Desc.NumArrayLayers = TexDesc.LayerCount;

		return Desc;
	}
};

struct RHITextureView
{
    virtual ~RHITextureView() = default;

    virtual const RHITextureViewDesc& GetDesc() const = 0;
    virtual RHITexture* GetRHITexture() const = 0;
    virtual void* GetNativeHandle() const = 0;
    virtual uint32_t GetBindingIndex() const = 0;
};

// Defines what happens to the memory when the Render Pass starts.
enum class LoadAction : uint8_t
{
	DontCare,   // Content is undefined (fastest, use if you overwrite everything)
	Load,       // Content is preserved from previous pass
	Clear       // Content is cleared to a specific value
};

// Defines what happens to the memory when the Render Pass ends.
enum class StoreAction : uint8_t
{
	DontCare,   // Content is discarded (use for transient depth buffers)
	Store,      // Content is saved to memory (standard)
	MultisampleResolve // Content is resolved to a non-MSAA texture
};

// Simple Color Helper
struct LinearColor
{
	float R, G, B, A;

	LinearColor() 
        : R(0), G(0), B(0), A(0) 
    {
    }
	
    LinearColor(float r, float g, float b, float a) 
        : R(r), G(g), B(b), A(a) 
    {
    }

	static const LinearColor Black;
	static const LinearColor White;
	static const LinearColor Red;
	static const LinearColor Green;
	static const LinearColor Blue;
};

// Helpers for static definitions
inline const LinearColor LinearColor::Black(0, 0, 0, 1);
inline const LinearColor LinearColor::White(1, 1, 1, 1);
inline const LinearColor LinearColor::Red(1, 0, 0, 1);
inline const LinearColor LinearColor::Green(0, 1, 0, 1);
inline const LinearColor LinearColor::Blue(0, 0, 1, 1);

static constexpr uint32_t MaxSimultaneousRenderTargets = 8;

struct RHIRenderTargetView
{
	// The specific view of the texture to render into
	RHITextureView* TextureView = nullptr;

	// Optional: If MSAA is used, this is the destination for the resolve
	RHITextureView* ResolveTargetView = nullptr;

	// What to do at start/end
	LoadAction LoadOp = LoadAction::DontCare;
	StoreAction StoreOp = StoreAction::Store;

	// The color to fill if LoadOp == Clear
	LinearColor ClearValue = LinearColor::Black;

	// Helper to check validity
	bool IsValid() const { return TextureView != nullptr; }
};

struct RHIDepthStencilView
{
	RHITextureView* TextureView = nullptr;

	// Depth Ops
	LoadAction DepthLoadOp = LoadAction::DontCare;
	StoreAction DepthStoreOp = StoreAction::Store;

	// Stencil Ops (often different from Depth)
	LoadAction StencilLoadOp = LoadAction::DontCare;
	StoreAction StencilStoreOp = StoreAction::Store;

	// Clear Values
	float ClearDepth = 0.0f; // Standard Z-up (0.0 = Near, 1.0 = Far) or Reverse-Z
	uint32_t ClearStencil = 0;

	bool IsValid() const { return TextureView != nullptr; }
};

struct RHIRenderPassInfo
{
	RHIRenderTargetView ColorRenderTargets[MaxSimultaneousRenderTargets];
	RHIDepthStencilView DepthStencilRenderTarget;

	// Tracks how many valid color targets are bound starting from index 0
	uint32_t NumColorRenderTargets = 0;

	// Default Constructor
	RHIRenderPassInfo()
	{
		// Zero initialization is usually safe due to default member initializers above
	}

	// Helper Constructor for a simple single-target pass
	explicit RHIRenderPassInfo(RHITextureView* InColorView, LoadAction InLoad = LoadAction::Load)
	{
		ColorRenderTargets[0].TextureView = InColorView;
		ColorRenderTargets[0].LoadOp = InLoad;
		ColorRenderTargets[0].StoreOp = StoreAction::Store;
		NumColorRenderTargets = 1;
	}

	// Helper Constructor for Color + Depth
	RHIRenderPassInfo(RHITextureView* InColorView, RHITextureView* InDepthView)
	{
		ColorRenderTargets[0].TextureView = InColorView;
		ColorRenderTargets[0].LoadOp = LoadAction::Load;
		NumColorRenderTargets = 1;

		DepthStencilRenderTarget.TextureView = InDepthView;
		DepthStencilRenderTarget.DepthLoadOp = LoadAction::Load;
	}
};