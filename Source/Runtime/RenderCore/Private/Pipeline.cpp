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
    Handle = GetRHI()->RHICreatePipelineStateObject(PipelineDesc);
}

FPipeline::~FPipeline()
{
    Shutdown();
}

void FPipeline::Init()
{
}

void FPipeline::Shutdown()
{
}
