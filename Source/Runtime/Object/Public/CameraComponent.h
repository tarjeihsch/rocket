#pragma once

#include "SceneComponent.h"

struct LRenderView;

class RCameraComponent : public RSceneComponent
{
    using Super = RSceneComponent;

public:
    RCameraComponent();

    static RClass* StaticClass();

    virtual RClass* GetClass() const;

    LRenderView BuildRenderView() const;

private:
    float FovY;

    float AspectRatio;

    float ZNear;

    float ZFar;
};