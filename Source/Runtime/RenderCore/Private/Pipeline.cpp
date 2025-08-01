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
    FAssetLoaderRequest VSRequest = { GAME_SRC "/Shader/VertexShader.hlsl" };
    ShaderAssetOptions VSOptions = { "main", "vs_6_6" };
    FAssetHandle<ShaderAsset> VertexShader = FAssetManager::Get().Load<ShaderAsset>(VSRequest, VSOptions);
    ShaderAsset* VertexShaderAsset = FAssetManager::Get().GetAsset(VertexShader);

    FAssetLoaderRequest PSRequest = { GAME_SRC "/Shader/FragmentShader.hlsl" };
    ShaderAssetOptions PSOptions = { "main", "ps_6_6" };
    FAssetHandle<ShaderAsset> PixelShader = FAssetManager::Get().Load<ShaderAsset>(PSRequest, PSOptions);
    ShaderAsset* PixelShaderAsset = FAssetManager::Get().GetAsset(PixelShader);

    FRHIPipelineDesc PipelineSpec;
    PipelineSpec.VSData = VertexShaderAsset->Data();
    PipelineSpec.VSWordCount = VertexShaderAsset->WordCount();
    PipelineSpec.PSData = PixelShaderAsset->Data();
    PipelineSpec.PSWordCount = PixelShaderAsset->WordCount();

    Handle = GetRHI()->RHICreatePipelineStateObject(PipelineSpec);

    FAssetManager::Get().Unload(VertexShader);
    FAssetManager::Get().Unload(PixelShader);
}

void FPipeline::Init()
{
    Compile();
}

void FPipeline::Shutdown()
{
}