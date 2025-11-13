#include "StaticMeshComponent.h"

#include "PrimitiveManager.h"
#include "World.h"

static glm::mat4 ComposeTransformMatrix(glm::vec3 Location, glm::vec3 Rotation, glm::vec3 Scale)
{
	glm::mat4 LocationMatrix = glm::translate(glm::mat4(1.0f), Location);
	glm::mat4 RotationMatrix = glm::toMat4(glm::quat(glm::radians(Rotation)));
	glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
	return LocationMatrix * RotationMatrix * ScaleMatrix;
}

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

    MarkDirty();
}

RStaticMesh *RStaticMeshComponent::GetStaticMesh() const
{
    return StaticMesh;
}

void RStaticMeshComponent::OnMarkedDirty()
{
    Super::OnMarkedDirty();

    SetVisibility(true);

    LSceneProxyGeometry Geometry = {};
    Geometry.StaticMesh = StaticMesh;

	LSceneProxyTransform Transform = {};
    Transform.ModelMatrix = ComposeTransformMatrix(Location, Rotation, Scale);

	GetWorld()->GetPrimitiveProxyManager()->UpdateProxyGeometry(ProxyHandle, Geometry);
	GetWorld()->GetPrimitiveProxyManager()->UpdateProxyTransform(ProxyHandle, Transform);
}