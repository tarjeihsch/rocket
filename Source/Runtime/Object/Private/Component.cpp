#include "Component.h"

RClass *RComponent::StaticClass()
{
    static RComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RComponent), alignof(RComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RComponent::RComponent()
    : bShouldTick(0),
    Owner(nullptr)
{
}
const RClass *RComponent::GetClass() const
{
    return StaticClass();
}