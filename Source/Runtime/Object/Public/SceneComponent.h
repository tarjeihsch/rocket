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

    glm::vec3 GetLocation() const;
    glm::vec3 GetRotation() const;

    void SetLocation(glm::vec3 Location);
    void SetRotation(glm::vec3 Rotation);

    void AddLocation(glm::vec3 Location);
    void AddRotation(glm::vec3 Rotation);

protected:
    void OnTransformChanged();

private:
    uint64_t SceneObjectID;

    glm::vec3 Location;

    glm::vec3 Rotation;

    glm::vec3 Scale;

    glm::vec3 Velocity;

    uint8_t bTransformDirty : 1;
};