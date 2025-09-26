#include "Texture2D.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"

FTexture2D::FTexture2D(uint32_t Width, uint32_t Height)
{
    FRHITextureDesc TextureDesc;
    TextureDesc.Format = ERHITextureFormat::B8G8R8A8_SRGB;
    TextureDesc.LayerCount = 1;
    TextureDesc.MipLevel = 1;
    TextureDesc.SampleCount = 1;
    TextureDesc.Lifetime = ERHIResourceLifetime::Transient;
    TextureDesc.Extent[0] = Width;
    TextureDesc.Extent[1] = Height;
    TextureDesc.Usage = ERHITextureUsage::SRV;
    TextureDesc.InitialLayout = ERHITextureLayout::ShaderReadOnly;

    Handle = GetRHI()->RHICreateTexture(TextureDesc);
}

FTexture2D::~FTexture2D()
{
    delete Handle;
}