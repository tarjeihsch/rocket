#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "Component.h"
#include "Transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

class CStaticMesh;

class RTransformComponent final : public RComponent
{
    using Super = RComponent;

public:
    static RClass* StaticClass();

    RTransformComponent();

    virtual const RClass* GetClass() const override;

    void SetTranslation(const glm::vec3 P_Translation) { Transform.M_Translation = P_Translation; }
    void SetRotation(const glm::vec3 P_Rotation) { Transform.M_Rotation = P_Rotation; }
    void SetScale(const glm::vec3 P_Scale) { Transform.M_Scale = P_Scale; }

    FTransform Transform;
};