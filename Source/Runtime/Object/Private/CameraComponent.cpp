#include "CameraComponent.h"

#include "EngineBase.h"
#include "../../Renderer/Public/PrimitiveManager.h"
#include "Viewport.h"

RCameraComponent::RCameraComponent()
    : bMarkedDirty(1)
{
    FovY = 60.0f;
    AspectRatio = 1280 / 720.0f;
    ZNear = 0.1f;
    ZFar = 1000.0f;
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

void RCameraComponent::PostConstruct()
{
    RSceneComponent::PostConstruct();
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
    LSceneView NewSceneView(SceneViewID, SceneViewID);

    bMarkedDirty = false;
}