#include "StaticMeshComponent.h"

RClass* RStaticMeshComponent::StaticClass()
{
    static RStaticMeshComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RStaticMeshComponent), alignof(RStaticMeshComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

const RClass *RStaticMeshComponent::GetClass() const
{
    return StaticClass();
}

void RStaticMeshComponent::SetStaticMesh(RStaticMesh *NewStaticMesh)
{
    StaticMesh = NewStaticMesh;
}

RStaticMesh *RStaticMeshComponent::GetStaticMesh() const
{
    return StaticMesh;
}