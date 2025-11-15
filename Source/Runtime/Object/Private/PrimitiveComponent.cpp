#include "PrimitiveComponent.h"

RClass* RPrimitiveComponent::StaticClass()
{
    static RPrimitiveComponent Archetype;

    static RClass Class
    {
        "RPrimitiveComponent",
        Super::StaticClass(),
        &Archetype,
        sizeof(RPrimitiveComponent),
        alignof(RPrimitiveComponent),
        C_ClassDefault | C_Memory_Allocator_Heap
    };

    return &Class;
}

RPrimitiveComponent::RPrimitiveComponent()
    : bMeshDirtyFlag(0)
{
}

const RClass* RPrimitiveComponent::GetClass() const
{
    return StaticClass();
}