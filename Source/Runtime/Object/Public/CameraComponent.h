#pragma once

#include "SceneComponent.h"

struct LRenderView;

enum class ECameraProjectionMode
{
	Perspective, Orthographic
};

class RCameraComponent : public RSceneComponent
{
    using Super = RSceneComponent;

public:
    static RClass* StaticClass();

    RCameraComponent();

    virtual RClass* GetClass() const override;
    virtual void PostConstruct(RWorld* World, RObject* Owner) override;
    virtual void Tick(float DeltaTime) override;

	glm::mat4 RecalculateViewMatrix() const;
	glm::mat4 RecalculateProjectionMatrix() const;

protected:
    void OnPropertyChanged();

public:
    uint64_t SceneViewID;

    float FovY;

    float AspectRatio;

    float ZNear;

    float ZFar;

	union
	{
		// Orthographic (Note: input must be in radians)
		float Zoom;

		// Perspective (Note: input must be in radians)
		float FovY;
	};

    ECameraProjectionMode ProjectionMode;

private:
    uint8_t bMarkedDirty : 1;
};