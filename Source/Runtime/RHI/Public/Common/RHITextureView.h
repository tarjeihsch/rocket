#pragma once

#include <cstdint>

#include "Color.h"
#include "Common/RHICommon.h"
#include "RHIDescriptorResource.h"

class RHITexture;

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
};

struct RHITextureView
{
	virtual ~RHITextureView() = default;

	virtual const RHITextureViewDesc& GetDesc() const = 0;
	virtual RHITexture* GetRHITexture() const = 0;
	virtual void* GetNativeHandle() const = 0;
	virtual RHIDescriptorHandle GetDescriptorHandle() const = 0;
};

enum class LoadAction : uint8_t
{
	// Content is undefined (fastest)
	DontCare,
	
	// Preserve content from previous pass
	Load,

	// Clear content from previous pass
	Clear
};

enum class StoreAction : uint8_t
{
	// Discard content (useful for transient depth buffers)
	DontCare,
	
	// Save content (standard)
	Store,
	
	// Resolve content to a non-MSAA texture
	MultisampleResolve
};

static constexpr uint32_t MaxSimultaneousRenderTargets = 8;

struct RHIRenderTargetView
{
	// The specific view of the texture to render into
	RHITextureView* TextureView = nullptr;

	// Optional: If MSAA is used, this is the destination for the resolve
	RHITextureView* ResolveTargetView = nullptr;

	LoadAction LoadOp = LoadAction::DontCare;

	StoreAction StoreOp = StoreAction::Store;

	LinearColor ClearValue = LinearColor::Black;

	inline bool IsValid() const 
	{ 
		return TextureView != nullptr; 
	}
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