#include "Attachment.h"

#include "Common/RHITexture.h"

FAttachment::FAttachment(uint32_t Width, uint32_t Height, ERHITextureFormat Format, ERHITextureUsage Usage)
{
    FRHITextureDesc TextureDesc;
    TextureDesc.Format = Format;
    TextureDesc.LayerCount = 1;
    TextureDesc.MipLevel = 1;
    TextureDesc.SampleCount = 1;
    TextureDesc.Lifetime = ERHIResourceLifetime::Transient;
    TextureDesc.Extent[0] = Width;
    TextureDesc.Extent[1] = Height;
    TextureDesc.Usage = Usage | ERHITextureUsage::SRV;
    TextureDesc.InitialLayout = ERHITextureLayout::Undefined;

    Handle = GetRHI()->RHICreateTexture(TextureDesc);
}

FAttachment::~FAttachment()
{
    delete Handle;
}
