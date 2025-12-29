#include "Material.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"
#include "Common/RHIPipeline.h"

RMaterial::RMaterial(EMaterialPass InPass, EMaterialBlendMode InBlendMode, const char* InVertexShaderPath, const char* InPixelShaderPath)
    : Pipeline(nullptr), VertexShaderPath(InVertexShaderPath), PixelShaderPath(InPixelShaderPath), Pass(InPass), BlendMode(InBlendMode)
{
    FRHIPipelineDesc Desc = {};
    Desc.VertexShaderPath = InVertexShaderPath;
    Desc.PixelShaderPath = InPixelShaderPath;
    Desc.ColorAttachmentFormats.push_back(ERHIFormat::BGRA8_SRGB);
	
    FRHIBlendAttachment& BlendAttachment = Desc.BlendAttachments.emplace_back();

    switch (InPass)
    {
    case EMaterialPass::Lit:
        Desc.CullMode = ERHICullMode::Back;
        Desc.bDepthWriteEnable = true;
        Desc.bDepthTestEnable = (InBlendMode == EMaterialBlendMode::Opaque || InBlendMode == EMaterialBlendMode::Masked);
		Desc.VertexBindings.push_back(FRHIVertexBinding(0, sizeof(FRHIVertex), 0));
		Desc.VertexAttributes.emplace_back(0, 0, ERHIFormat::Float3, 0);
		Desc.VertexAttributes.emplace_back(1, 0, ERHIFormat::Float3, 12);
		Desc.VertexAttributes.emplace_back(2, 0, ERHIFormat::Float2, 24);

        break;
	
    case EMaterialPass::DepthOnly:
		Desc.CullMode = ERHICullMode::Back;
		Desc.bDepthTestEnable = true;
		Desc.bDepthWriteEnable = true;
		Desc.VertexBindings.push_back(FRHIVertexBinding(0, sizeof(FRHIVertex), 0));
		Desc.VertexAttributes.emplace_back(0, 0, ERHIFormat::Float3, 0);

        break;
    
    case EMaterialPass::Shadow:
        Desc.CullMode = ERHICullMode::Front;
		Desc.bDepthTestEnable = true;
		Desc.bDepthWriteEnable = true;
		Desc.VertexBindings.push_back(FRHIVertexBinding(0, sizeof(FRHIVertex), 0));
		Desc.VertexAttributes.emplace_back(0, 0, ERHIFormat::Float3, 0);

        break;
    
    case EMaterialPass::UI:
        Desc.CullMode = ERHICullMode::None;
        Desc.bDepthTestEnable = false;
        Desc.bDepthWriteEnable = false;
		Desc.VertexBindings.push_back(FRHIVertexBinding(0, 20, 0));
		Desc.VertexAttributes.emplace_back(0, 0, ERHIFormat::Float2, 0);
		Desc.VertexAttributes.emplace_back(1, 0, ERHIFormat::Float2, 8);
		Desc.VertexAttributes.emplace_back(2, 0, ERHIFormat::UNorm8x4, 16);

        break;
    }
    
    switch (InBlendMode)
    {
	case EMaterialBlendMode::Opaque:
	case EMaterialBlendMode::Masked:
        BlendAttachment.bEnableBlend = false;
		break;
        
    case EMaterialBlendMode::Additive:
        BlendAttachment.bEnableBlend = true;
        BlendAttachment.SrcColorFactor = ERHIBlendFactor::SrcAlpha;
        BlendAttachment.DstColorFactor = ERHIBlendFactor::One;
        BlendAttachment.ColorOp = ERHIBlendOp::Add;
        BlendAttachment.SrcAlphaFactor = ERHIBlendFactor::One;
        BlendAttachment.DstAlphaFactor = ERHIBlendFactor::One;
        BlendAttachment.AlphaOp = ERHIBlendOp::Add;
        break;
   
    case EMaterialBlendMode::Transparent:
        BlendAttachment.bEnableBlend = true;
        BlendAttachment.SrcColorFactor = ERHIBlendFactor::SrcAlpha;
        BlendAttachment.DstColorFactor = ERHIBlendFactor::OneMinusSrcAlpha;
        BlendAttachment.ColorOp = ERHIBlendOp::Add;
        BlendAttachment.SrcAlphaFactor = ERHIBlendFactor::One;
        BlendAttachment.DstAlphaFactor = ERHIBlendFactor::OneMinusSrcAlpha;
        BlendAttachment.AlphaOp = ERHIBlendOp::Add;
		break;
    }

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

void RMaterial::Bind(RHICommandList& CmdList)
{
    GetRHI()->RHICmdBindPipeline(CmdList, Pipeline);
}