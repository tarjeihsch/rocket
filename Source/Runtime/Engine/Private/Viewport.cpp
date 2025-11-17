#include "Viewport.h"

#include "FrameGraph.h"
#include "RenderTarget.h"
#include "EngineBase.h"
#include "Renderer.h"

RViewport::RViewport()
{
}

RViewport::~RViewport()
{
}

void RViewport::Update(float DeltaTime)
{
    RenderTarget.reset(new FRenderTarget(1280, 720));
}

void RViewport::Present()
{
    GEngine->GetRenderer()->Present(this);
}