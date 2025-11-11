#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/vec3.hpp"
#include "FrameGraph.h"

struct FRenderTarget;
class IRenderer;

enum class ERenderViewProjectionMode
{
    Perspective, Orthographic
};

struct LRect2D
{
    float Offset[2];
    float Extent[2];
};

struct LRenderView
{
    // Location in world space
    alignas(16)
    glm::vec3 Location;

    // Rotation in euler angles
    alignas(16)
    glm::vec3 Rotation;

    // Offset and extent of the viewport
    LRect2D ViewRect;

    // Viewing volume
    ERenderViewProjectionMode ProjectionMode;

    float ZNear;
    float ZFar;
    float AspectRatio;

    union
    {
        // Orthographic (Note: input must be in radians)
        float Zoom;

        // Perspective (Note: input must be in radians)
        float FovY;
    };

    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjection() const;
};

class FRHITexture;
class RViewportClient;

// Rendering, audio, input systems
class RViewport
{
public:
    RViewport();
    ~RViewport();

    void Update(RViewportClient* ViewportClient, float DeltaTime);

private:
    void Init();
    void Shutdown();
    
private:
    FRHITexture* RenderTarget;
};

struct RFrameContext
{
    float ViewportWidth;
    float ViewportHeight;
    float Timestep;

	FFrameGraphBuilder Builder;
};

class RViewportClient
{
public:
	virtual void Draw(RFrameContext& Context) = 0;
};