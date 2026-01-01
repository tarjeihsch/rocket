#include "Texture2D.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"

Texture2D::Texture2D(ETextureType InType, uint32_t InWidth, uint32_t InHeight)
    : Type(InType), Width(InWidth), Height(InHeight)
{
	RHITextureDesc TextureDesc;

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
	TextureDesc.Lifetime = RHIResourceLifetime::Persistent;
	TextureDesc.extent[0] = Width;
	TextureDesc.extent[1] = Height;
	TextureDesc.Usage = ERHITextureUsage::SRV | ERHITextureUsage::CopyDst;
	TextureDesc.initial_layout = RHITextureLayout::ShaderReadOnly;

    Handle = GetRHI()->rhi_create_texture(TextureDesc);
}

Texture2D::~Texture2D()
{
    delete Handle;
}

void Texture2D::LoadRawTextureData(const uint8_t* Data)
{
//    GetRHI()->RHIUploadTextureBlocking(Handle, Data, Width, Height);
}
