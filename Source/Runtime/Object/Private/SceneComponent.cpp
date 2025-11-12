#include "SceneComponent.h"

#include "EngineBase.h"
#include "../../Renderer/Public/PrimitiveManager.h"

RClass *RSceneComponent::StaticClass()
{
    static RSceneComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RSceneComponent), alignof(RSceneComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RSceneComponent::RSceneComponent()
    : Location(0),
    Rotation(0),
    Scale(1),
    Velocity(0)
{
}

const RClass *RSceneComponent::GetClass() const
{
    return StaticClass();
}

glm::vec3 RSceneComponent::GetLocation() const
{
    return Location;
}

glm::vec3 RSceneComponent::GetRotation() const
{
    return Rotation;
}

void RSceneComponent::SetLocation(glm::vec3 InLocation)
{
    Location = InLocation;

    OnMarkedDirty();
}

void RSceneComponent::SetRotation(glm::vec3 InRotation)
{
    Rotation = InRotation;

    OnMarkedDirty();
}

void RSceneComponent::AddLocation(glm::vec3 InLocation)
{
    Location += InLocation;

    OnMarkedDirty();
}

void RSceneComponent::AddRotation(glm::vec3 InRotation)
{
    Rotation += InRotation;

    OnMarkedDirty();
}