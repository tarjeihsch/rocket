#include "SceneRenderer.h"

#include "Attachment.h"
#include "Common/RHI.h"
#include "Common/RHICommandList.h"
#include "Camera.h"
#include "FrameGraph.h"
#include "Pipeline.h"
#include "PrimitiveDrawInterface.h"
#include "RenderTarget.h"
#include "Scene.h"
#include "StaticMesh.h"
#include "Texture2D.h"
#include "Common/RHIBuffer.h"
#include "Common/RHIDrawCommand.h"
#include "glm/gtc/matrix_inverse.hpp"

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

    FPipeline Pipeline = FPipeline(GAME_SRC "/Shader/VertexShader.hlsl", GAME_SRC "/Shader/FragmentShader.hlsl");
    FRenderTarget RenderTarget = FRenderTarget(1280, 720);
    FAttachment DepthAttachment = FAttachment(1280, 720, ERHITextureFormat::D32_SFLOAT_S8_UINT, ERHITextureUsage::Depth | ERHITextureUsage::Stencil);

    FFrameGraphBuilder Builder;

    FFrameGraphPass& DrawPass = Builder.AddPass("Draw Static Mesh", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBindPipeline(RHICmdList, Pipeline.GetHandle());
        Scene->GetRegistry()->View<FStaticMeshComponent, FTransformComponent>([&](const FStaticMeshComponent& StaticMeshComponent, const FTransformComponent& TransformComponent)
        {
            struct FInstanceData
            {
                glm::mat4 Transform;
                glm::mat4 TransformInverseTranspose;
            };

            FCameraData CameraData = Scene->GetCamera()->GetCameraData();
            FInstanceData InstanceData = { TransformComponent.ToMatrix(), glm::inverseTranspose(TransformComponent.ToMatrix()) };

            std::shared_ptr<FRHIBuffer> CameraGPUData = GetRHI()->RHIBindBuffer(&CameraData, sizeof(FCameraData));
            std::shared_ptr<FRHIBuffer> InstanceGPUData = GetRHI()->RHIBindBuffer(&InstanceData, sizeof(FInstanceData));

            FRHIDrawCommand DrawCommand = {};
            DrawCommand.VertexBuffer = StaticMeshComponent.Handle->GetVertexBuffer();
            DrawCommand.IndexBuffer = StaticMeshComponent.Handle->GetIndexBuffer();
            DrawCommand.IndexCount = StaticMeshComponent.Handle->GetIndexCount();

            FRHIDrawParams DrawParams = {};
            DrawParams.DrawParam_1 = CameraGPUData->GetBindingIndex(); // Camera ID
            DrawParams.DrawParam_2 = InstanceGPUData->GetBindingIndex(); // Instance ID

            GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);
        });
    });
    DrawPass.AddColorAttachment(RenderTarget.GetHandle());
    DrawPass.AddDepthAttachment(DepthAttachment.GetHandle());

    FFrameGraphPass& CompositionPass = Builder.AddPass("Composition", EFGPassFlags::Compute, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBlitImage(RHICmdList, RenderTarget.GetHandle(), GetRHI()->RHIGetBackBufferRenderTarget());
    });
    CompositionPass.AddTransferSrc(RenderTarget.GetHandle());
    CompositionPass.AddTransferDst(GetRHI()->RHIGetBackBufferRenderTarget());

    FFrameGraphPass& PresentPass = Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
    PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());

    Builder.Execute();
}