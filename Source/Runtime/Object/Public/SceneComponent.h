#pragma once

#include "Component.h"
#include "Transform.h"
#include "Vector.h"

class RSceneComponent : public RComponent
{
    using Super = RObject;

public:
    static RClass* StaticClass();

    RSceneComponent();

    virtual const RClass* GetClass() const override;

    virtual void Tick(float DeltaTime) override;

    FTransform Transform;

    glm::vec3 Velocity;

    uint8_t bShouldTick : 1;
};