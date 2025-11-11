#pragma once

#include "Renderer.h"

class RViewport;
class FRHITexture;

class REditorRenderer
{
public:
    REditorRenderer();
    ~REditorRenderer();

    virtual void Init();
    virtual void Shutdown();
    virtual void Render(float DeltaTime);

private:
	FRHITexture* ImGuiTextureAtlas;
};