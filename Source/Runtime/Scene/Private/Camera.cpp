#include "Camera.h"

void CFreeLookCamera::Capture(const FCameraCaptureSettings& CaptureData)
{
    switch (CaptureData.ProjectionMode)
    {
        case ECameraProjectionMode::Orthographic:
            CachedCaptureResult.Projection = glm::ortho(-CaptureData.AspectRatio * CaptureData.Zoom, CaptureData.AspectRatio * CaptureData.Zoom, -1.0f * CaptureData.Zoom, 1.0f * CaptureData.Zoom, CaptureData.ZNear, CaptureData.ZFar);
            CachedCaptureResult.Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
            CachedCaptureResult.Projection[3][1] *= -1.0f; // Invert Y-Axis Translation
            break;
        case ECameraProjectionMode::Perspective:
            CachedCaptureResult.Projection = glm::perspective(CaptureData.FovY, CaptureData.AspectRatio, CaptureData.ZNear, CaptureData.ZFar);
            CachedCaptureResult.Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
            break;
    }

    glm::vec3 Forward;
    Forward.x = glm::cos(CaptureData.Rotation.x) * glm::cos(CaptureData.Rotation.y);
    Forward.y = glm::sin(CaptureData.Rotation.x);
    Forward.z = glm::cos(CaptureData.Rotation.x) * glm::sin(CaptureData.Rotation.y);
    const glm::vec3 Direction = glm::normalize(Forward);
    const glm::vec3 Right = glm::normalize(glm::cross(Direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 Up    = glm::cross(Right, Direction);

    CachedCaptureResult.ViewMatrix = glm::lookAt(CaptureData.Position, CaptureData.Position + Direction, Up);
}

const FCameraCaptureResult& CFreeLookCamera::GetCachedCapture() const
{
    return CachedCaptureResult;
}