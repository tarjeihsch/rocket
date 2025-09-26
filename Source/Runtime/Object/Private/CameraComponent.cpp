#include "CameraComponent.h"

#include "Viewport.h"

RCameraComponent::RCameraComponent()
{
    FovY = 60.0f;
    AspectRatio = 1280 / 720.0f;
    ZNear = 0.1f;
    ZFar = 1000.0f;
}

RClass * RCameraComponent::StaticClass()
{
    static RCameraComponent Archetype;

    static RClass Class = RClass(Super::StaticClass(), &Archetype, sizeof(RCameraComponent), alignof(RCameraComponent), C_ClassDefault | C_Memory_Allocator_Heap);

    return &Class;
}

RClass *RCameraComponent::GetClass() const
{
    return StaticClass();
}

LRenderView RCameraComponent::BuildRenderView() const
{
    LRenderView RenderView;
    RenderView.FovY = FovY;
    RenderView.AspectRatio = AspectRatio;
    RenderView.ZNear = ZNear;
    RenderView.ZFar = ZFar;
    RenderView.Location = GetLocation();
    RenderView.Rotation = GetRotation();
    return RenderView;
}