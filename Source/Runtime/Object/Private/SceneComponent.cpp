#include "SceneComponent.h"

RClass *RSceneComponent::StaticClass()
{
    static RComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RSceneComponent), alignof(RSceneComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RSceneComponent::RSceneComponent()
    : bShouldTick(1)
{
}

const RClass *RSceneComponent::GetClass() const
{
    return StaticClass();
}

void RSceneComponent::Tick(float DeltaTime)
{
    Transform.M_Translation += Velocity * DeltaTime;
}