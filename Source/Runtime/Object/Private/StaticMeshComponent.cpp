#include "StaticMeshComponent.h"

#include "EngineBase.h"
#include "Scene.h"
#include "../../Renderer/Public/SceneRenderer.h"
#include "StaticMesh.h"

RClass *RStaticMeshComponent::StaticClass()
{
    static RStaticMeshComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RStaticMeshComponent), alignof(RStaticMeshComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RStaticMeshComponent::RStaticMeshComponent()
    : bDirty(false)
{
}

const RClass *RStaticMeshComponent::GetClass() const
{
    return StaticClass();
}

void RStaticMeshComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDirty)
    {
        OnMeshChanged();
    }
}

void RStaticMeshComponent::SetStaticMesh(RStaticMesh *NewStaticMesh)
{
    StaticMesh = NewStaticMesh;

    bDirty = true;
}

RStaticMesh *RStaticMeshComponent::GetStaticMesh() const
{
    return StaticMesh;
}

void RStaticMeshComponent::OnMeshChanged()
{
    GEngine->GetScene()->UpdateSceneObjectGeometry(SceneObjectID, StaticMesh);
    bDirty = false;
}