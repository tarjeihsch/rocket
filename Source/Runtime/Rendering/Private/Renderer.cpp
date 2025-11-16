#include "Renderer.h"

#include "Assertion.h"
#include "Material.h"
#include "RenderTarget.h"
#include "Texture2D.h"

RRenderer::RRenderer()
{
}

RRenderer::~RRenderer()
{
}

void RRenderer::DrawIndexed(std::span<FRHIDrawCall> Commands)
{
	FFrameGraphBuilder Builder;

    FRenderTarget ColorRT = FRenderTarget(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);

    auto& Pass = Builder.AddPass("Pass", EFGPassFlags::Graphics | EFGPassFlags::Dynamic | EFGPassFlags::ClearColor, [&](FRHICommandList& RHICmdList)
    {
        for (const auto& DrawCommand : Commands)
        {
            DrawCommand.Material->Bind(RHICmdList);

            GetRHI()->RHICmdDrawIndexed2(RHICmdList, DrawCommand);
        }
    });
    Pass.AddColorAttachment(ColorRT.GetHandle());

    FFrameGraphPass& CompositionPass = Builder.AddPass("Composition", EFGPassFlags::Compute, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBlitImage(RHICmdList, ColorRT.GetHandle(), GetRHI()->RHIGetBackBufferRenderTarget());
    });
    CompositionPass.AddTransferSrc(ColorRT.GetHandle());
    CompositionPass.AddTransferDst(GetRHI()->RHIGetBackBufferRenderTarget());

    FFrameGraphPass& PresentPass = Builder.AddPass("Present", EFGPassFlags::Graphics, nullptr);
    PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());

    Builder.Execute();
}
