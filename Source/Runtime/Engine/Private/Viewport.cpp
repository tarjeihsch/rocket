#include "Viewport.h"

#include "Common/RHI.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "RenderTarget.h"
#include <Assertion.h>

glm::mat4 LRenderView::GetViewMatrix() const
{
    glm::vec3 Forward;
    Forward.x = glm::cos(Rotation.x) * glm::cos(Rotation.y);
    Forward.y = glm::sin(Rotation.x);
    Forward.z = glm::cos(Rotation.x) * glm::sin(Rotation.y);
    const glm::vec3 Direction = glm::normalize(Forward);
    const glm::vec3 Right = glm::normalize(glm::cross(Direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 Up    = glm::cross(Right, Direction);

    return glm::lookAt(Location, Location + Direction, Up);
}

glm::mat4 LRenderView::GetProjection() const
{
    glm::mat4 Projection = {};

    switch (ProjectionMode)
    {
    case ERenderViewProjectionMode::Orthographic:
        Projection = glm::ortho(-AspectRatio * Zoom, AspectRatio * Zoom, -1.0f * Zoom, 1.0f * Zoom, ZNear, ZFar);
        Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
        Projection[3][1] *= -1.0f; // Invert Y-Axis Translation
        break;
    case ERenderViewProjectionMode::Perspective:
        Projection = glm::perspective(FovY, AspectRatio, ZNear, ZFar);
        Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
        break;
    }
    return Projection;
}

RViewport::RViewport()
{
}

RViewport::~RViewport()
{
    Shutdown();
}

void RViewport::Init()
{
}

void RViewport::Shutdown()
{
}

void RViewport::Update(RViewportClient* ViewportClient, float DeltaTime)
{
    RFrameContext FrameCtx = {};
    FrameCtx.ViewportWidth = GetRHI()->RHIGetViewportExtent()[0];
    FrameCtx.ViewportHeight = GetRHI()->RHIGetViewportExtent()[1];
    FrameCtx.Timestep = DeltaTime;

    ViewportClient->Draw(FrameCtx);
}