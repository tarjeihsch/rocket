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
		TextureDesc.format = RHITextureFormat::R8G8B8A8_SRGB;
		break;
	case ETextureType::Linear:
		TextureDesc.format = RHITextureFormat::R8G8B8A8_UNORM;
		break;
    }

	TextureDesc.format = RHITextureFormat::R8G8B8A8_UNORM;
	TextureDesc.LayerCount = 1;
	TextureDesc.NumMips = 1;
	TextureDesc.SampleCount = 1;
	TextureDesc.lifetime = RHIResourceLifetime::Persistent;
	TextureDesc.extent[0] = Width;
	TextureDesc.extent[1] = Height;
	TextureDesc.usage = RHITextureUsage::SRV | RHITextureUsage::CopyDst;
	TextureDesc.initial_layout = RHITextureLayout::ShaderReadOnly;

    Handle = GetRHI()->RHICreateTexture(TextureDesc);
}

Texture2D::~Texture2D()
{
    delete Handle;
}

void Texture2D::LoadRawTextureData(const uint8_t* Data)
{
//    GetRHI()->RHIUploadTextureBlocking(Handle, Data, Width, Height);
}
