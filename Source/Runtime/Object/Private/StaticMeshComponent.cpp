#include "StaticMeshComponent.h"

#include "SceneRenderer.h"
#include "StaticMesh.h"

RClass *RStaticMeshComponent::StaticClass()
{
    static RStaticMeshComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RStaticMeshComponent), alignof(RStaticMeshComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RStaticMeshComponent::RStaticMeshComponent()
{

}

RStaticMeshComponent::~RStaticMeshComponent()
{
    delete StaticMesh;
}

void RStaticMeshComponent::OnInit()
{
    RComponent::OnInit();
}
void RStaticMeshComponent::OnDestroy()
{
    RComponent::OnDestroy();
}

const RClass *RStaticMeshComponent::GetClass() const
{
    return StaticClass();
}

void RStaticMeshComponent::SetStaticMesh(CStaticMesh *NewStaticMesh)
{
    FSceneRenderer::Get().RegisterProxy(NewStaticMesh, &Transform);

    StaticMesh = NewStaticMesh;
}
