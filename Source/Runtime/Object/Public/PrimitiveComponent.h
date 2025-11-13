#pragma once

#include "SceneComponent.h"

class RPrimitiveComponent : public RSceneComponent
{
    using Super = RSceneComponent;

public:
    static RClass* StaticClass();

    RPrimitiveComponent();

    virtual const RClass* GetClass() const override;

protected:
    uint64_t ProxyHandle = UINT64_MAX;

protected:
    uint8_t bMeshDirtyFlag : 1;
};
