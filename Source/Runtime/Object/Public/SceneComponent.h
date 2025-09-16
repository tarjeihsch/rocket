#pragma once

#include "Component.h"
#include "Transform.h"
#include "glm/vec3.hpp"

class RSceneComponent : public RComponent
{
    using Super = RObject;

public:
    static RClass* StaticClass();

    RSceneComponent();

    virtual const RClass* GetClass() const override;

    FTransform Transform;
};