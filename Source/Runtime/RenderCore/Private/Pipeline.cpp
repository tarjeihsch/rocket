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

FPipeline::~FPipeline()
{
    Shutdown();
}

void FPipeline::Compile()
{
    FAssetLoaderRequest VSRequest = { VertexShaderPath };
    ShaderAssetOptions VSOptions = { "main", "vs_6_6" };
    FAssetHandle<ShaderAsset> VertexShader = FAssetManager::Get().Load<ShaderAsset>(VSRequest, VSOptions);
    ShaderAsset* VertexShaderAsset = FAssetManager::Get().GetAsset(VertexShader);

    FAssetLoaderRequest PSRequest = { FragmentShaderPath };
    ShaderAssetOptions PSOptions = { "main", "ps_6_6" };
    FAssetHandle<ShaderAsset> PixelShader = FAssetManager::Get().Load<ShaderAsset>(PSRequest, PSOptions);
    ShaderAsset* PixelShaderAsset = FAssetManager::Get().GetAsset(PixelShader);

    FRHIPipelineDesc PipelineSpec;
    PipelineSpec.PixelShader = PixelShaderAsset;
    PipelineSpec.VertexShader = VertexShaderAsset;
    PipelineSpec.bDepthTestEnable = true;
    PipelineSpec.bDepthWriteEnable = true;
    PipelineSpec.DepthFormat = ERHIFormat::D32_Float_S8X24;
    PipelineSpec.CullMode = ERHICullMode::Back;
    PipelineSpec.ColorAttachmentFormats.push_back(ERHIFormat::BGRA8_SRGB);
    PipelineSpec.VertexBindings.push_back(FRHIVertexBinding {
        .Binding = 0,
        .Stride = 48,
        .bPerInstance = 0
    });
    PipelineSpec.VertexAttributes.push_back({
        .Location = 0,
        .Binding = 0,
        .Format = ERHIFormat::Float3,
        .Offset = offsetof(FRHIVertex, m_Position)
    });
    PipelineSpec.VertexAttributes.push_back({
        .Location = 1,
        .Binding = 0,
        .Format = ERHIFormat::Float3,
        .Offset = offsetof(FRHIVertex, m_Normal)
    });
    PipelineSpec.VertexAttributes.push_back({
        .Location = 2,
        .Binding = 0,
        .Format = ERHIFormat::Float2, // Should map to VK_FORMAT_R32G32_SFLOAT
        .Offset = offsetof(FRHIVertex, m_TexCoord)
    });

    Handle = GetRHI()->RHICreatePipelineStateObject(PipelineSpec);

    FAssetManager::Get().Unload(VertexShader);
    FAssetManager::Get().Unload(PixelShader);
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
