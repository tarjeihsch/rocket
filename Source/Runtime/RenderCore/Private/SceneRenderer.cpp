#include "SceneRenderer.h"

#include "../Public/FrameGraph.h"
#include "Buffer.h"
#include "Common/RHI.h"
#include "Common/RHICommandList.h"
#include "Pipeline.h"
#include "PrimitiveDrawInterface.h"
#include "RenderTarget.h"
#include "Scene.h"
#include "StaticMesh.h"
#include "Texture2D.h"

void FSceneRenderer::Init()
{

}

void FSceneRenderer::Tick(PScene* Scene)
{
    // Traverse the scene
    // Gather all Static Mesh Components
    // Add each ID from SMC to an array of FMeshDrawCommand (eg. DrawCommands)
    // At the end, call RenderGraph.AddPass([&](FRHICommandList& CmdList)
    // {
    //      CmdList.Enqueue<FRHIDraw>(DrawCommands)
    // });

    // Apply the same concept for Instanced Static Mesh Components, same for IndexedStaticMeshComponents and IndexedIndirectStaticMeshComponent

    std::vector<FMeshDrawCommand> DrawCommands;

    Scene->GetRegistry()->View<FStaticMeshComponent>([&](const FStaticMeshComponent& StaticMeshComponent)
    {
        DrawCommands.push_back({ StaticMeshComponent.Handle });
    });

    FVertexBuffer VertexBuffer = FVertexBuffer(1024);
    FPipeline Pipeline = FPipeline("VertexShader.hlsl", "PixelShader.hlsl");
    FRenderTarget RenderTarget = FRenderTarget(1280, 720);

    std::vector<uint32_t> TestData = { 0xDEADBEEF, 0x12345678, 0xCAFEBABE, 0x0BADF00D };
    uint64_t DataSize = TestData.size() * sizeof(uint32_t);
    VertexBuffer.UploadDataBlocking(TestData.data(), DataSize);

    FFrameGraphBuilder Builder;

    FFrameGraphPass& DrawPass = Builder.AddPass("Draw Static Mesh", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBindPipeline(RHICmdList, Pipeline.GetHandle());
        GetRHI()->RHICmdDraw(RHICmdList, DrawCommands);
    });
    DrawPass.AddColorAttachment(RenderTarget.GetHandle());

    FFrameGraphPass& PresentPass = Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
    PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());

    Builder.Execute();
}