#include "Renderer.h"

#include "RenderTarget.h"
#include "FrameGraph.h"
#include "Material.h"
#include "Primitive.h"
#include "Common/RHI.h"
#include "Common/RHIBuffer.h"

RSceneRenderer* GRenderer = nullptr;

void RSceneRenderer::AddItem(RPrimitive *Primitive, RMaterial *Material, RTransform *Transform)
{
    LDrawItem DrawCommand = {};
    DrawCommand.Primitive = Primitive;
    DrawCommand.Transform = Transform;
    ItemData.push_back(DrawCommand);
}

void RSceneRenderer::Render()
{
    FFrameGraphBuilder Builder;

    FRenderTarget RenderTarget = FRenderTarget(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);

    Render_Opaque_Internal(Builder);

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

void RSceneRenderer::Render_Opaque_Internal(FFrameGraphBuilder& Builder)
{
    Builder.AddPass("Opaque", EFGPassFlags::Graphics | EFGPassFlags::Dynamic, [&](FRHICommandList& RHICmdList)
    {
        for (const LDrawView& View : ViewData)
        {
            FRHIBufferMemoryPacker CameraData(ERHIBufferMemoryPackerAlignmentLayout::STD430);
            CameraData.Write(View.ViewMatrix);
            CameraData.Write(View.ProjectionMatrix);
            std::shared_ptr<FRHIBuffer> ViewBuffer = GetRHI()->RHIBindBuffer(CameraData.GetData(), CameraData.GetSize());

            for (const LDrawItem& Item : ItemData)
            {
                Item.Primitive->Draw(RHICmdList);
            }
        }
    });
}
