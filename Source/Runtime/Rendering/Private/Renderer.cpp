#include "Renderer.h"

#include "Assertion.h"
#include "FrameGraph.h"
#include "Material.h"
#include "RenderTarget.h"
#include "Texture2D.h"
#include "Viewport.h"

RRenderer::RRenderer()
{
}

RRenderer::~RRenderer()
{
}

void RRenderer::BeginFrame()
{
    FrameGraph.reset(new FFrameGraphBuilder());
}

void RRenderer::EndFrame()
{
    FrameGraph->Execute();
}

void RRenderer::DrawIndexed(RViewport* Viewport, std::span<FRHIDrawCall> Commands)
{
    ensure(Viewport->RenderTarget != nullptr);

    DrawIndexed(Viewport->RenderTarget, Commands);
}

void RRenderer::DrawIndexed(RViewport* Viewport, const FRHIDrawCall& Commands)
{
    ensure(Viewport->RenderTarget != nullptr);

    auto& Pass = FrameGraph->AddPass("Pass", EFGPassFlags::Graphics | EFGPassFlags::Dynamic | EFGPassFlags::ClearColor, [&](FRHICommandList& RHICmdList)
    {
        Commands.Material->Bind(RHICmdList);

        GetRHI()->RHICmdDrawIndexed2(RHICmdList, Commands);
    });
    Pass.AddColorAttachment(Viewport->RenderTarget->GetHandle());
}

void RRenderer::DrawIndexed(const std::shared_ptr<FRenderTarget>& RenderTarget, std::span<FRHIDrawCall> Commands)
{
    ensure(RenderTarget != nullptr);

    auto& Pass = FrameGraph->AddPass("Pass", EFGPassFlags::Graphics | EFGPassFlags::Dynamic | EFGPassFlags::ClearColor, [&](FRHICommandList& RHICmdList)
    {
        for (const auto& DrawCommand : Commands)
        {
            DrawCommand.Material->Bind(RHICmdList);

            GetRHI()->RHICmdDrawIndexed2(RHICmdList, DrawCommand);
        }
    });
    Pass.AddColorAttachment(RenderTarget->GetHandle());
}

void RRenderer::Present(RViewport* Viewport)
{
    Present(Viewport->RenderTarget);
}

void RRenderer::Present(const std::shared_ptr<FRenderTarget>& RenderTarget)
{
    ensure(RenderTarget != nullptr);

    FFrameGraphPass& CompositionPass = FrameGraph->AddPass("Composition", EFGPassFlags::Compute, [&](FRHICommandList& RHICmdList)
    {
        GetRHI()->RHICmdBlitImage(RHICmdList, RenderTarget->GetHandle(), GetRHI()->RHIGetBackBufferRenderTarget());
    });
    CompositionPass.AddTransferSrc(RenderTarget->GetHandle());
    CompositionPass.AddTransferDst(GetRHI()->RHIGetBackBufferRenderTarget());

    FFrameGraphPass& PresentPass = FrameGraph->AddPass("Present", EFGPassFlags::Graphics, nullptr);
    PresentPass.AddPresentSrc(GetRHI()->RHIGetBackBufferRenderTarget());
}