#pragma once

#include "MulticastDelegate.h"
#include "Common/RHIDrawCommand.h"

#include <span>
#include <memory>

class RViewport;
class FRenderTarget;
class FFrameGraphBuilder;

using TOnFrameBegin = TMulticastDelegate<FFrameGraphBuilder&>;
using TOnFrameEnd = TMulticastDelegate<FFrameGraphBuilder&>;

// Renderer acts as a high-level wrapper over the RHI render graph
class RRenderer final
{
public:
    TOnFrameBegin OnFrameBegin;
    TOnFrameEnd OnFrameEnd;

    RRenderer();
    ~RRenderer();

    void DrawIndexed(RViewport* Viewport, const FRHIDrawCall& Commands);
    void DrawIndexed(RViewport* Viewport, std::span<FRHIDrawCall> Commands);
    void DrawIndexed(const std::shared_ptr<FRenderTarget>& RenderTarget, std::span<FRHIDrawCall> Commands);

    void Present(RViewport* Viewport);
    void Present(const std::shared_ptr<FRenderTarget>& RenderTarget);

    void BeginFrame();
    void EndFrame();

private:
    std::unique_ptr<FFrameGraphBuilder> FrameGraph;
};