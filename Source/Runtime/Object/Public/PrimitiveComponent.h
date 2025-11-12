#pragma once

#include "SceneComponent.h"

class RPrimitiveComponent : public RSceneComponent
{
    using Super = RSceneComponent;

public:
    static RClass* StaticClass();

    RPrimitiveComponent();

    virtual const RClass* GetClass() const override;
    virtual void Tick(float DeltaTime) override;

    void SetVisibility(bool bVisibility);

protected:
    virtual void OnMarkedDirty() override;

protected:
    uint64_t ProxyHandle;

private:
    uint8_t bDirty : 1;

    uint8_t bVisible : 1;
};
