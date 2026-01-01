#include "RenderTarget.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"

FRenderTarget::FRenderTarget(uint32_t Width, uint32_t Height)
{
    RHITextureDesc TextureDesc;
    TextureDesc.Format = ERHITextureFormat::B8G8R8A8_SRGB;
    TextureDesc.LayerCount = 1;
    TextureDesc.MipLevel = 1;
    TextureDesc.SampleCount = 1;
    TextureDesc.Lifetime = RHIResourceLifetime::Transient;
    TextureDesc.extent[0] = Width;
    TextureDesc.extent[1] = Height;
    TextureDesc.Usage = ERHITextureUsage::Color | ERHITextureUsage::SRV | ERHITextureUsage::CopySrc;
    TextureDesc.initial_layout = RHITextureLayout::None;

    Handle = GetRHI()->rhi_create_texture(TextureDesc);
}

FRenderTarget::~FRenderTarget()
{
    delete Handle;
}