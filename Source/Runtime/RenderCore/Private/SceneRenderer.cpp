#include "SceneRenderer.h"

#include "Attachment.h"
#include "Common/RHI.h"
#include "Common/RHICommandList.h"
#include "Camera.h"
#include "FrameGraph.h"
#include "Math.h"
#include "Pipeline.h"
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

    FPipeline Pipeline = FPipeline(GAME_SRC "/Shader/VertexShader.hlsl", GAME_SRC "/Shader/FragmentShader.hlsl");
    FRenderTarget RenderTarget = FRenderTarget(1280, 720);
    FAttachment DepthAttachment = FAttachment(1280, 720, ERHITextureFormat::D32_SFLOAT_S8_UINT, ERHITextureUsage::Depth | ERHITextureUsage::Stencil);

    FFrameGraphBuilder Builder;

    FFrameGraphPass& DrawPass = Builder.AddPass("Draw Static Mesh", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBindPipeline(RHICmdList, Pipeline.GetHandle());
        Scene->GetRegistry()->View<StaticMeshComponent, TransformComponent>([&](const StaticMeshComponent& P_StaticMeshComponent, const TransformComponent& P_TransformComponent)
        {
            struct FInstanceData
            {
                glm::mat4 Transform;
                glm::mat4 TransformInverseTranspose;
            };

            FCameraCaptureResult CameraData = Scene->GetActiveCamera()->GetCachedCapture();

            std::shared_ptr<FRHIBuffer> CameraGPUData = GetRHI()->RHIBindBuffer(&CameraData, sizeof(FCameraCaptureResult));

            for (const auto& Section : P_StaticMeshComponent.Handle->GetMeshSections())
            {
                Transform transform = P_TransformComponent.M_Transform;
                transform.M_Translation += Section.M_Translation * 2.0f;
                transform.M_Rotation += Section.M_Rotation;
                transform.M_Scale += Section.M_Scale;

                FInstanceData InstanceData = { transform.ToMatrix(), glm::inverseTranspose(transform.ToMatrix()) };

                std::shared_ptr<FRHIBuffer> InstanceGPUData = GetRHI()->RHIBindBuffer(&InstanceData, sizeof(FInstanceData));

                FRHIDrawCommand DrawCommand = {};
                DrawCommand.VertexBuffer = P_StaticMeshComponent.Handle->GetVertexBuffer();
                DrawCommand.IndexBuffer = P_StaticMeshComponent.Handle->GetIndexBuffer();
                DrawCommand.IndexCount = Section.M_IndexCount;
                DrawCommand.VertexBufferOffset = Section.M_VertexOffset;
                DrawCommand.IndexBufferOffset = Section.M_IndexOffset;

                FRHIDrawParams DrawParams = {};
                DrawParams.DrawParam_1 = CameraGPUData->GetBindingIndex(); // Camera ID
                DrawParams.DrawParam_2 = InstanceGPUData->GetBindingIndex(); // Instance ID

                GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);
            }
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