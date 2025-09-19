#include "Pipeline.h"

#include "AssetManager.h"
#include "Common/RHI.h"
#include "Common/RHIPipeline.h"
#include "ShaderAsset.h"

FPipeline::FPipeline(const char* InVertexShaderPath, const char* InFragmentShaderPath)
    : VertexShaderPath(InVertexShaderPath), FragmentShaderPath(InFragmentShaderPath)
{
    Init();
}

FPipeline::FPipeline(FRHIPipelineDesc PipelineDesc)
{
    PipelineDesc.bDepthTestEnable = false;
    PipelineDesc.bDepthWriteEnable = false;
    PipelineDesc.CullMode = ERHICullMode::None;
    PipelineDesc.ColorAttachmentFormats.push_back(ERHIFormat::BGRA8_SRGB);
    Handle = GetRHI()->RHICreatePipelineStateObject(PipelineDesc);
}

FPipeline::~FPipeline()
{
    Shutdown();
}

void FPipeline::Compile()
{
    FRHIPipelineDesc PipelineSpec = {};
    PipelineSpec.PixelShaderPath = FragmentShaderPath;
    PipelineSpec.VertexShaderPath = VertexShaderPath;
    PipelineSpec.bDepthTestEnable = true;
    PipelineSpec.bDepthWriteEnable = true;
    PipelineSpec.DepthFormat = ERHIFormat::D32_Float_S8X24;
    PipelineSpec.CullMode = ERHICullMode::Back;
    PipelineSpec.ColorAttachmentFormats.push_back(ERHIFormat::BGRA8_SRGB);
    //PipelineSpec.VertexBindings.push_back(FRHIVertexBinding(0, 48, 0));
    //PipelineSpec.VertexAttributes.push_back(FRHIVertexAttribute(0, 0, ERHIFormat::Float3, offsetof(FRHIVertex, m_Position)));
    //PipelineSpec.VertexAttributes.push_back(FRHIVertexAttribute(1, 0, ERHIFormat::Float3, offsetof(FRHIVertex, m_Normal)));
    //PipelineSpec.VertexAttributes.push_back(FRHIVertexAttribute(2, 0, ERHIFormat::Float2, offsetof(FRHIVertex, m_TexCoord)));

    Handle = GetRHI()->RHICreatePipelineStateObject(PipelineSpec);
}

std::shared_ptr<FRHIBuffer> FPipeline::BindBuffer(void *Data, uint64_t Size)
{
    return GetRHI()->RHIBindBuffer(Data, Size);
}

void FPipeline::Init()
{
    Compile();
}

void FPipeline::Shutdown()
{
}
