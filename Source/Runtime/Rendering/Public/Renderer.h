#pragma once

#include "FrameGraph.h"
#include "MulticastDelegate.h"
#include "Common/RHIDrawCommand.h"

#include <span>

class RPrimitive;

using TOnFrameBegin = TMulticastDelegate<>;
using TOnFrameDraw = TMulticastDelegate<FFrameGraphBuilder&>;
using TOnFrameEnd = TMulticastDelegate<>;

// Renderer acts as a high-level wrapper over the RHI render graph
class RRenderer final
{
public:
    TOnFrameBegin OnFrameBegin;
    TOnFrameDraw OnFrameDraw;
    TOnFrameEnd OnFrameEnd;

    RRenderer();
    ~RRenderer();

    void DrawIndexed(std::span<FRHIDrawCall> Commands);
};