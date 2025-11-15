#include "StaticMeshComponent.h"

#include "Scene.h"
#include "World.h"

static glm::mat4 ComposeTransformMatrix(const glm::vec3& Location, const glm::vec3& Rotation, const glm::vec3& Scale)
{
	glm::mat4 LocationMatrix = glm::translate(glm::mat4(1.0f), Location);
	glm::mat4 RotationMatrix = glm::toMat4(glm::quat(glm::radians(Rotation)));
	glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
	return LocationMatrix * RotationMatrix * ScaleMatrix;
}

static glm::mat4 TransposeInverseTransformMatrix(const glm::vec3& Location, const glm::vec3& Rotation, const glm::vec3& Scale)
{
	glm::mat4 LocationMatrix = glm::translate(glm::mat4(1.0f), Location);
	glm::mat4 RotationMatrix = glm::toMat4(glm::quat(glm::radians(Rotation)));
	glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
    glm::mat4 Model = LocationMatrix * RotationMatrix * ScaleMatrix;
	glm::mat4 NormalMatrix = glm::transpose(glm::inverse(Model));
    return NormalMatrix;
}

RClass* RStaticMeshComponent::StaticClass()
{
    static RStaticMeshComponent Archetype;

    static RClass Class
	{
	    "RStaticMeshComponent",
		Super::StaticClass(), 
		&Archetype, 
		sizeof(RStaticMeshComponent), 
		alignof(RStaticMeshComponent), 
		C_ClassDefault | C_Memory_Allocator_Heap
	};

    return &Class;
}

void RStaticMeshComponent::Tick(float DeltaTime)
{
    RPrimitiveComponent::Tick(DeltaTime);

    if (bMeshDirtyFlag || bTransformDirtyFlag)
    {
        OnPropertyChanged();
    }
}

const RClass *RStaticMeshComponent::GetClass() const
{
    return StaticClass();
}

void RStaticMeshComponent::OnPropertyChanged()
{
    if (ProxyHandle == UINT64_MAX)
    {
        ProxyHandle = GetWorld()->GetSceneProxyManager()->CreateProxy();
    }

    LSceneProxyGeometry Geometry = {};
    Geometry.StaticMesh = StaticMesh;

    LSceneProxyTransform Transform = {};
    Transform.ModelMatrix = ComposeTransformMatrix(Location, Rotation, Scale);
    Transform.InverseModelMatrix = TransposeInverseTransformMatrix(Location, Rotation, Scale);

    GetWorld()->GetSceneProxyManager()->UpdateProxyGeometry(ProxyHandle, Geometry);
    GetWorld()->GetSceneProxyManager()->UpdateProxyTransform(ProxyHandle, Transform);
}

void RStaticMeshComponent::SetStaticMesh(RStaticMesh *NewStaticMesh)
{
    StaticMesh = NewStaticMesh;

    OnPropertyChanged();
}

RStaticMesh *RStaticMeshComponent::GetStaticMesh() const
{
    return StaticMesh;
}