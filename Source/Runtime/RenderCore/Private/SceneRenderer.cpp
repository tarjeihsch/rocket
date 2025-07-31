#include "SceneRenderer.h"

#include "PrimitiveDrawInterface.h"
#include "Scene.h"
#include "StaticMesh.h"
#include "Common/RHICommandList.h"
#include "Common/FrameGraph.h"
#include "Common/RHI.h"

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
        DrawCommands.push_back({ StaticMeshComponent.VertexBuffer, StaticMeshComponent.IndexBuffer });
    });

    std::unique_ptr<FRHITexture> RenderTarget(GetRHI()->RHICreateTexture(FRHITextureDesc::Create2D(1280, 720, ERHIResourceMemory::Transient, ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureUsage::Color, ERHITextureLayout::Undefined, 0, 0, 1)));

    FFrameGraphBuilder Builder;
    FFrameGraphPass& DrawPass = Builder.AddPass("Draw Static Mesh", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdDraw(RHICmdList, DrawCommands);
    });
    DrawPass.AddColorAttachment(RenderTarget.get());

    FFrameGraphPass& PresentPass = Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
    PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());

    Builder.Execute();

}