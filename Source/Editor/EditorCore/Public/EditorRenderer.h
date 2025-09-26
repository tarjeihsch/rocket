#pragma once

#include "Renderer.h"

class REditorRenderer : public IRenderer
{
public:
    virtual void Init() override;
    virtual void Shutdown() override;
    virtual void Tick(float DeltaTime) override;
};