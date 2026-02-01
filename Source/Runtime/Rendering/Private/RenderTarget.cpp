#include "RenderTarget.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"
#include "Common/RHITextureView.h"

RenderTarget::RenderTarget(const Vector2i& extent)
{
    RHITextureDesc color_texture_desc = {};
    color_texture_desc.format = RHITextureFormat::R8G8B8A8_UNORM;
    color_texture_desc.LayerCount = 1;
    color_texture_desc.NumMips = 1;
    color_texture_desc.SampleCount = 1;
    color_texture_desc.lifetime = RHIResourceLifetime::Transient;
    color_texture_desc.extent[0] = extent.x;
    color_texture_desc.extent[1] = extent.y;
    color_texture_desc.usage = RHITextureUsage::Color | RHITextureUsage::SRV | RHITextureUsage::CopySrc;
    color_texture_desc.initial_layout = RHITextureLayout::ColorAttachment;
    rhi_texture_color = GetRHI()->RHICreateTexture(color_texture_desc);

    RHITextureDesc depth_texture_desc = {};
    depth_texture_desc.format = RHITextureFormat::D32_SFLOAT_S8_UINT;
    depth_texture_desc.LayerCount = 1;
    depth_texture_desc.NumMips = 1;
    depth_texture_desc.SampleCount = 1;
    depth_texture_desc.lifetime = RHIResourceLifetime::Transient;
    depth_texture_desc.extent[0] = extent.x;
    depth_texture_desc.extent[1] = extent.y;
    depth_texture_desc.usage = RHITextureUsage::Depth | RHITextureUsage::Stencil;
    depth_texture_desc.initial_layout = RHITextureLayout::DepthStencil;
    rhi_texture_depth = GetRHI()->RHICreateTexture(depth_texture_desc);

	RHITextureViewDesc texture_view_color_desc = {};
	texture_view_color_desc.Lifetime = RHIResourceLifetime::Transient;
	texture_view_color_desc.aspect = RHITextureAspect::Color;
	texture_view_color_desc.Format = RHITextureFormat::R8G8B8A8_UNORM;
    rhi_texture_view_color = GetRHI()->RHICreateTextureView(texture_view_color_desc, rhi_texture_color);

	RHITextureViewDesc texture_view_depth_stencil_desc = {};
	texture_view_depth_stencil_desc.Lifetime = RHIResourceLifetime::Transient;
	texture_view_depth_stencil_desc.aspect = RHITextureAspect::DepthStencil;
	texture_view_depth_stencil_desc.Format = RHITextureFormat::D32_SFLOAT_S8_UINT;
	rhi_texture_view_depth = GetRHI()->RHICreateTextureView(texture_view_depth_stencil_desc, rhi_texture_depth);
}

RenderTarget::~RenderTarget()
{
    delete rhi_texture_color;
    delete rhi_texture_depth;
    delete rhi_texture_view_color;
    delete rhi_texture_view_depth;
}