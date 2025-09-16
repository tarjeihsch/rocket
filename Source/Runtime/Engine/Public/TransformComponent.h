#pragma once

#include "Component.h"
#include "glm/mat4x4.hpp"

class CStaticMesh;

class RTransformComponent final : public RComponent
{
    using Super = RComponent;

public:
    static RClass* StaticClass();

    RTransformComponent();

    virtual const RClass* GetClass() const override;

private:
    glm::mat4 Transform;
};