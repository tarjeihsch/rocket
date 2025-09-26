#pragma once

#include "Renderer.h"

class RSceneRenderer : public IRendererInterface
{
public:
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void Render(std::span<LSceneObject> View) override;
};