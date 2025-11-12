#pragma once

#include "SceneComponent.h"

struct LRenderView;

class RCameraComponent : public RSceneComponent
{
    using Super = RSceneComponent;

public:
    static RClass* StaticClass();

    RCameraComponent();

    virtual RClass* GetClass() const override;
    virtual void PostConstruct() override;
    virtual void Tick(float DeltaTime) override;

protected:
    void OnPropertyChanged();

public:
    uint64_t SceneViewID;

    float FovY;

    float AspectRatio;

    float ZNear;

    float ZFar;

private:
    uint8_t bMarkedDirty : 1;
};