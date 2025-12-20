#include "RenderTarget.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"

FRenderTarget::FRenderTarget(uint32_t Width, uint32_t Height)
{
    FRHITextureDesc TextureDesc;
    TextureDesc.Format = ERHITextureFormat::B8G8R8A8_SRGB;
    TextureDesc.LayerCount = 1;
    TextureDesc.MipLevel = 1;
    TextureDesc.SampleCount = 1;
    TextureDesc.Lifetime = ERHIResourceLifetime::Transient;
    TextureDesc.Extent[0] = Width;
    TextureDesc.Extent[1] = Height;
    TextureDesc.Usage = ERHITextureUsage::Color | ERHITextureUsage::SRV | ERHITextureUsage::CopySrc;
    TextureDesc.InitialLayout = ERHITextureLayout::Undefined;

    Handle = GetRHI()->RHICreateTexture(TextureDesc);
}

FRenderTarget::~FRenderTarget()
{
    delete Handle;
}