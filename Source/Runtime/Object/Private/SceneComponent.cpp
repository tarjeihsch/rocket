#include "SceneComponent.h"

#include "EngineBase.h"

RClass *RSceneComponent::StaticClass()
{
    static RSceneComponent Archetype;

    static RField Fields[] =
    {
        RK_FIELD(glm::vec3, RSceneComponent, Location),
        RK_FIELD(glm::vec3, RSceneComponent, Rotation),
        RK_FIELD(glm::vec3, RSceneComponent, Scale)
    };

    static RClass Class
    {
        "RSceneComponent",
        Super::StaticClass(),
        &Archetype,
        sizeof(RSceneComponent),
        alignof(RSceneComponent),
        C_ClassDefault | C_Memory_Allocator_Heap,
        Fields,
        std::size(Fields)
    };

    return &Class;
}

RSceneComponent::RSceneComponent()
    : Location(0),
    Rotation(0),
    Scale(1),
    Velocity(0),
    bTransformDirtyFlag(0)
{
}

const RClass *RSceneComponent::GetClass() const
{
    return StaticClass();
}

void RSceneComponent::OnPropertyChanged()
{
    RComponent::OnPropertyChanged();
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

    bTransformDirtyFlag = true;
}

void RSceneComponent::SetRotation(glm::vec3 InRotation)
{
    Rotation = InRotation;

    bTransformDirtyFlag = true;
}

void RSceneComponent::AddLocation(glm::vec3 InLocation)
{
    Location += InLocation;

    bTransformDirtyFlag = true;
}

void RSceneComponent::AddRotation(glm::vec3 InRotation)
{
    Rotation += InRotation;

    bTransformDirtyFlag = true;
}