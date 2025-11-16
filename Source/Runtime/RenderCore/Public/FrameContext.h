#pragma once
#include "FrameGraph.h"

// A transient object that should never outlive a single frame.
// Responsible for wrapping and executing the render graph.
class RFrameContext
{
public:
    std::vector<std::function<void()>> Callback;

    // Translates recorded render passes into RHI commands, dispatch work
    void Dispatch();

private:
    FFrameGraphBuilder Builder;
};

inline void RFrameContext::Dispatch()
{
    for (auto& Func : Callback)
    {
        Func();
    }

    Builder.Execute();
}

#define ENQUEUE_RENDER_PASS()