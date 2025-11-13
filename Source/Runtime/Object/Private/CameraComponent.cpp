#include "CameraComponent.h"

#include "EngineBase.h"
#include "../../Renderer/Public/PrimitiveManager.h"
#include "Viewport.h"
#include "World.h"

glm::mat4 RCameraComponent::RecalculateViewMatrix() const
{
	glm::vec3 Forward;
	Forward.x = glm::cos(Rotation.x) * glm::cos(Rotation.y);
	Forward.y = glm::sin(Rotation.x);
	Forward.z = glm::cos(Rotation.x) * glm::sin(Rotation.y);
	const glm::vec3 Direction = glm::normalize(Forward);
	const glm::vec3 Right = glm::normalize(glm::cross(Direction, glm::vec3(0.0f, 1.0f, 0.0f)));
	const glm::vec3 Up = glm::cross(Right, Direction);

	return glm::lookAt(Location, Location + Direction, Up);
}

glm::mat4 RCameraComponent::RecalculateProjectionMatrix() const
{
	glm::mat4 Projection = {};

	switch (ProjectionMode)
	{
	case ECameraProjectionMode::Orthographic:
		Projection = glm::ortho(-AspectRatio * Zoom, AspectRatio * Zoom, -1.0f * Zoom, 1.0f * Zoom, ZNear, ZFar);
		Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
		Projection[3][1] *= -1.0f; // Invert Y-Axis Translation
		break;
	case ECameraProjectionMode::Perspective:
		Projection = glm::perspective(FovY, AspectRatio, ZNear, ZFar);
		Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
		break;
	}
	return Projection;
}

RCameraComponent::RCameraComponent()
    : bMarkedDirty(1)
{
    FovY = glm::radians(60.0f);
    AspectRatio = 1280 / 720.0f;
    ZNear = 0.1f;
    ZFar = 1000.0f;
	ProjectionMode = ECameraProjectionMode::Perspective;
	Zoom = 1.0f;
}

RClass* RCameraComponent::StaticClass()
{
    static RCameraComponent Archetype;

    static RClass Class = RClass(Super::StaticClass(), &Archetype, sizeof(RCameraComponent), alignof(RCameraComponent), C_ClassDefault | C_Memory_Allocator_Heap);

    return &Class;
}

RClass* RCameraComponent::GetClass() const
{
    return StaticClass();
}

void RCameraComponent::PostConstruct(RWorld* World, RObject* Owner)
{
    Super::PostConstruct(World, Owner);

	SceneViewID = GetWorld()->GetSceneProxyManager()->CreateView();
}

void RCameraComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bMarkedDirty)
    {
        OnPropertyChanged();
    }
}

void RCameraComponent::OnPropertyChanged()
{
	Super::OnMarkedDirty();

    LSceneView NewSceneView(SceneViewID, SceneViewID);

	NewSceneView.ProjectionMatrix = RecalculateProjectionMatrix();
	NewSceneView.ViewMatrix = RecalculateViewMatrix();

    GetWorld()->GetSceneProxyManager()->UpdateView(SceneViewID, NewSceneView);
}