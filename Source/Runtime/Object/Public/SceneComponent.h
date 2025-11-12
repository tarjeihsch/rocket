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

    glm::vec3 GetLocation() const;
    glm::vec3 GetRotation() const;

    void SetLocation(glm::vec3 Location);
    void SetRotation(glm::vec3 Rotation);

    void AddLocation(glm::vec3 Location);
    void AddRotation(glm::vec3 Rotation);

protected:
    virtual void OnMarkedDirty() {}

private:
    glm::vec3 Location;

    glm::vec3 Rotation;

    glm::vec3 Scale;

    glm::vec3 Velocity;
};