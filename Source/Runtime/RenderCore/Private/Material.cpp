#include "Material.h"

#include "Common/RHI.h"

RMaterial::RMaterial(const char* VertexShader, const char* PixelShaderPath)
    : Pipeline(nullptr)
{
    FRHIPipelineDesc Desc;
    Desc.CullMode = ERHICullMode::Back;
    Desc.VertexShaderPath = VertexShader;
    Desc.PixelShaderPath = PixelShaderPath;
    Desc.PolygonMode = ERHIPolygonMode::Fill;

    Pipeline = GetRHI()->RHICreatePipelineStateObject(Desc);
}

RMaterial::~RMaterial()
{
}

void RMaterial::SetBufferParameter(const std::string &Name, RBuffer* Buffer)
{
}

void RMaterial::SetTextureParameter(const std::string &Name, RTexture2D* Texture)
{
}

void RMaterial::Bind()
{

}