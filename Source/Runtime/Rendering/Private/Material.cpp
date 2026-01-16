#include "Material.h"

#include "Common/RHI.h"
#include "Common/RHITexture.h"
#include "Common/RHIPipeline.h"

RMaterial::RMaterial(EMaterialPass InPass, EMaterialBlendMode InBlendMode, const char* InVertexShaderPath, const char* InPixelShaderPath)
    : Pipeline(nullptr), VertexShaderPath(InVertexShaderPath), PixelShaderPath(InPixelShaderPath), Pass(InPass), BlendMode(InBlendMode)
{
    RHIPipelineDesc Desc = {};
    Desc.vertex_shader_path = InVertexShaderPath;
    Desc.pixel_shader_path = InPixelShaderPath;
    Desc.color_attachment_formats.push_back(RHIFormat::BGRA8_SRGB);
	
    RHIBlendAttachment& BlendAttachment = Desc.BlendAttachments.emplace_back();

    switch (InPass)
    {
    case EMaterialPass::Lit:
        Desc.cull_mode = RHICullMode::Back;
        Desc.depth_write_enable = true;
        Desc.depth_test_enable = (InBlendMode == EMaterialBlendMode::Opaque || InBlendMode == EMaterialBlendMode::Masked);
		Desc.VertexBindings.push_back(RHIVertexBinding(0, sizeof(RHIVertex), 0));
		Desc.VertexAttributes.emplace_back(0, 0, RHIFormat::Float3, 0);
		Desc.VertexAttributes.emplace_back(1, 0, RHIFormat::Float3, 12);
		Desc.VertexAttributes.emplace_back(2, 0, RHIFormat::Float2, 24);

        break;
	
    case EMaterialPass::DepthOnly:
		Desc.cull_mode = RHICullMode::Back;
		Desc.depth_test_enable = true;
		Desc.depth_write_enable = true;
		Desc.VertexBindings.push_back(RHIVertexBinding(0, sizeof(RHIVertex), 0));
		Desc.VertexAttributes.emplace_back(0, 0, RHIFormat::Float3, 0);

        break;
    
    case EMaterialPass::Shadow:
        Desc.cull_mode = RHICullMode::Front;
		Desc.depth_test_enable = true;
		Desc.depth_write_enable = true;
		Desc.VertexBindings.push_back(RHIVertexBinding(0, sizeof(RHIVertex), 0));
		Desc.VertexAttributes.emplace_back(0, 0, RHIFormat::Float3, 0);

        break;
    
    case EMaterialPass::UI:
        Desc.cull_mode = RHICullMode::None;
        Desc.depth_test_enable = false;
        Desc.depth_write_enable = false;
		Desc.VertexBindings.push_back(RHIVertexBinding(0, 20, 0));
		Desc.VertexAttributes.emplace_back(0, 0, RHIFormat::Float2, 0);
		Desc.VertexAttributes.emplace_back(1, 0, RHIFormat::Float2, 8);
		Desc.VertexAttributes.emplace_back(2, 0, RHIFormat::UNorm8x4, 16);

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
        BlendAttachment.SrcColorFactor = RHIBlendFactor::SrcAlpha;
        BlendAttachment.DstColorFactor = RHIBlendFactor::One;
        BlendAttachment.ColorOp = RHIBlendOp::Add;
        BlendAttachment.SrcAlphaFactor = RHIBlendFactor::One;
        BlendAttachment.DstAlphaFactor = RHIBlendFactor::One;
        BlendAttachment.AlphaOp = RHIBlendOp::Add;
        break;
   
    case EMaterialBlendMode::Transparent:
        BlendAttachment.bEnableBlend = true;
        BlendAttachment.SrcColorFactor = RHIBlendFactor::SrcAlpha;
        BlendAttachment.DstColorFactor = RHIBlendFactor::OneMinusSrcAlpha;
        BlendAttachment.ColorOp = RHIBlendOp::Add;
        BlendAttachment.SrcAlphaFactor = RHIBlendFactor::One;
        BlendAttachment.DstAlphaFactor = RHIBlendFactor::OneMinusSrcAlpha;
        BlendAttachment.AlphaOp = RHIBlendOp::Add;
		break;
    }

    Pipeline = GetRHI()->RHICreatePipeline(Desc);
}

RMaterial::~RMaterial()
{
}

void RMaterial::SetBufferParameter(const std::string &Name, RBuffer* Buffer)
{
}

void RMaterial::SetTextureParameter(const std::string &Name, Texture2D* Texture)
{
}

void RMaterial::Bind(RHICommandList& CmdList)
{
//    GetRHI()->RHICmdBindPipeline(CmdList, Pipeline);
}