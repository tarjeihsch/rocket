#include "SceneRenderer.h"

#include "Attachment.h"
#include "Camera.h"
#include "Common/RHI.h"
#include "Common/RHIBuffer.h"
#include "Common/RHICommandList.h"
#include "Common/RHIDrawCommand.h"
#include "FrameGraph.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "StaticMesh.h"
#include "Transform.h"
#include "glm/gtc/matrix_inverse.hpp"

void RSceneRenderer::Init()
{

}

void RSceneRenderer::Tick(float DeltaTime)
{
    // Traverse the scene
    // Gather all Static Mesh Components
    // Add each ID from SMC to an array of FMeshDrawCommand (eg. DrawCommands)
    // At the end, call RenderGraph.AddPass([&](FRHICommandList& CmdList)
    // {
    //      CmdList.Enqueue<FRHIDraw>(DrawCommands)
    // });

    // Apply the same concept for Instanced Static Mesh Components, same for IndexedStaticMeshComponents and IndexedIndirectStaticMeshComponent

    FPipeline Pipeline = FPipeline(ROCKET_SRC "/Shader/VertexShader.hlsl", ROCKET_SRC "/Shader/FragmentShader.hlsl");
    FRenderTarget RenderTarget = FRenderTarget(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);
    FAttachment DepthAttachment = FAttachment(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1], ERHITextureFormat::D32_SFLOAT_S8_UINT, ERHITextureUsage::Depth | ERHITextureUsage::Stencil);

    FFrameGraphBuilder Builder;

    FFrameGraphPass& DrawPass = Builder.AddPass("Draw Static Mesh", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBindPipeline(RHICmdList, Pipeline.GetHandle());

        if (!Camera)
        {
            return;
        }

        FRHIBufferMemoryPacker CameraData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
        CameraData.Write(Camera->GetCachedCapture().ViewMatrix);
        CameraData.Write(Camera->GetCachedCapture().Projection);
        std::shared_ptr<FRHIBuffer> CameraInterface_GPUBuffer = GetRHI()->RHIBindBuffer(CameraData.GetData(), CameraData.GetSize());

        for (auto Proxy : Proxies)
        {
            CStaticMesh* StaticMesh = Proxy.StaticMesh;
            FTransform* Transform = Proxy.Transform;

            for (const auto& Section : StaticMesh->GetMeshSections())
            {
                const glm::mat4 ModelMatrix = Transform->ToMatrix() * Section.M_LocalMatrix;

                FRHIBufferMemoryPacker ObjectData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
                ObjectData.Write(ModelMatrix);
                ObjectData.Write(glm::inverseTranspose(ModelMatrix));
                std::shared_ptr<FRHIBuffer> ObjectInterface_GPUBuffer = GetRHI()->RHIBindBuffer(ObjectData.GetData(), ObjectData.GetSize());

                FRHIDrawCommand DrawCommand = {};
                DrawCommand.VertexBuffer = StaticMesh->GetVertexBuffer();
                DrawCommand.IndexBuffer = StaticMesh->GetIndexBuffer();
                DrawCommand.IndexCount = Section.M_IndexCount;
                DrawCommand.VertexBufferOffset = Section.M_VertexOffset;
                DrawCommand.IndexBufferOffset = Section.M_IndexOffset;

                FRHIDrawParams DrawParams = {};
                DrawParams.DrawParam_1 = CameraInterface_GPUBuffer->GetBindingIndex(); // Camera ID
                DrawParams.DrawParam_2 = ObjectInterface_GPUBuffer->GetBindingIndex(); // Instance ID

                GetRHI()->RHICmdDrawIndexed(RHICmdList, DrawCommand, DrawParams);
            }
        }
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

void RSceneRenderer::RegisterProxy(CStaticMesh *StaticMesh, FTransform* Transform)
{
    RRenderProxy Proxy = { StaticMesh, Transform};

    Proxies.push_back(Proxy);
}
void RSceneRenderer::SetCamera(ICamera *InCamera)
{
    Camera = InCamera;

    RK_LOG_INFO("asdasdasd");
}
