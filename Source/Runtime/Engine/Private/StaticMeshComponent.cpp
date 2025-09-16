#include "StaticMeshComponent.h"

RClass *RStaticMeshComponent::StaticClass()
{
    static RComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RComponent), alignof(RComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RStaticMeshComponent::RStaticMeshComponent()
    : RenderProxy()
{
}

const RClass *RStaticMeshComponent::GetClass() const
{
    return StaticClass();
}

void RStaticMeshComponent::SetStaticMesh(CStaticMesh *NewStaticMesh)
{
    RenderProxy = { NewStaticMesh };
}

CStaticMesh *RStaticMeshComponent::GetStaticMesh() const
{
    return RenderProxy.Handle;
}