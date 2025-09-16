#include "TransformComponent.h"

#include "Transform.h"

RClass *RTransformComponent::StaticClass()
{
    static RComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RComponent), alignof(RComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RTransformComponent::RTransformComponent()
    : Transform()
{
}

const RClass *RTransformComponent::GetClass() const
{
    return StaticClass();
}