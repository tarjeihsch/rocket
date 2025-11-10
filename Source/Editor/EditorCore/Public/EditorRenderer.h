#pragma once

#include "Renderer.h"

class RViewport;

class REditorRenderer
{
public:
    virtual void Init();
    virtual void Shutdown();
    virtual void Render(RViewport* Viewport);
};