#include "Texture2D.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"

RTexture2D::RTexture2D(ETextureType InType, uint32_t InWidth, uint32_t InHeight)
    : Type(InType), Width(InWidth), Height(InHeight)
{
	FRHITextureDesc TextureDesc;

    switch (Type)
    {
    case ETextureType::SRGB:
		TextureDesc.Format = ERHITextureFormat::R8G8B8A8_SRGB;
		break;
	case ETextureType::Linear:
		TextureDesc.Format = ERHITextureFormat::R8G8B8A8_UNORM;
		break;
    }

	TextureDesc.Format = ERHITextureFormat::R8G8B8A8_UNORM;
	TextureDesc.LayerCount = 1;
	TextureDesc.MipLevel = 1;
	TextureDesc.SampleCount = 1;
	TextureDesc.Lifetime = ERHIResourceLifetime::Persistent;
	TextureDesc.Extent[0] = Width;
	TextureDesc.Extent[1] = Height;
	TextureDesc.Usage = ERHITextureUsage::SRV | ERHITextureUsage::CopyDst;
	TextureDesc.InitialLayout = ERHITextureLayout::ShaderReadOnly;

    Handle = GetRHI()->RHICreateTexture(TextureDesc);
}

RTexture2D::~RTexture2D()
{
    delete Handle;
}

void RTexture2D::LoadRawTextureData(const uint8_t* Data)
{
    GetRHI()->RHIUploadTextureBlocking(Handle, Data, Width, Height);
}
