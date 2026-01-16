#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "Math/Vector2.h"
#include "Math/Vector4.h"

struct RHITexture;
struct RHITextureView;

enum class LoadOp { Load, Clear, DontCare };
enum class StoreOp { Store, DontCare };

struct ClearColorValue
{
    Vector4i color_value;
};

struct ClearDepthStencilValue
{
    float depth_value;
    uint32_t stencil_value;
};

struct RenderTarget
{
    RenderTarget(const Vector2i& in_extent);
    ~RenderTarget();

    inline RHITexture* GetColorRHITexture() const;
    inline RHITexture* GetDepthRHITexture() const;

    inline RHITextureView* GetColorRHITextureView() const;
    inline RHITextureView* GetDepthRHITextureView() const;

private:
    RHITexture* rhi_texture_color;
    RHITexture* rhi_texture_depth;

    RHITextureView* rhi_texture_view_color;
    RHITextureView* rhi_texture_view_depth;
};

inline RHITexture* RenderTarget::GetColorRHITexture() const
{
    return rhi_texture_color;
}

inline RHITexture* RenderTarget::GetDepthRHITexture() const
{
    return rhi_texture_depth;
}

inline RHITextureView* RenderTarget::GetColorRHITextureView() const
{
    return rhi_texture_view_color;
}

inline RHITextureView* RenderTarget::GetDepthRHITextureView() const
{
    return rhi_texture_view_depth;
}