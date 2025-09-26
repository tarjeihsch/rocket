#include "SceneComponent.h"

#include "EngineBase.h"
#include "Scene.h"

RClass *RSceneComponent::StaticClass()
{
    static RComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RSceneComponent), alignof(RSceneComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RSceneComponent::RSceneComponent()
    : Location(0),
    Rotation(0),
    Scale(1),
    Velocity(0),
    bTransformDirty(false)
{
}

const RClass *RSceneComponent::GetClass() const
{
    return StaticClass();
}

void RSceneComponent::Tick(float DeltaTime)
{
    if (bTransformDirty)
    {
        OnTransformChanged();
    }
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

    bTransformDirty = true;
}

void RSceneComponent::SetRotation(glm::vec3 InRotation)
{
    Rotation = InRotation;

    bTransformDirty = true;
}

void RSceneComponent::AddLocation(glm::vec3 InLocation)
{
    Location += InLocation;

    bTransformDirty = true;
}

void RSceneComponent::AddRotation(glm::vec3 InRotation)
{
    Rotation += InRotation;
}

void RSceneComponent::OnTransformChanged()
{
    GEngine->GetScene()->UpdateSceneObjectTransform(SceneObjectID, Location, Rotation, Scale);

    bTransformDirty = false;
}