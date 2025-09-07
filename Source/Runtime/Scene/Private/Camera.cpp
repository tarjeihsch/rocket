#include "Camera.h"

CPerspectiveCamera::CPerspectiveCamera()
{
     Settings.ProjectionMode = ECameraProjectionMode::Perspective;
     Settings.FoVY = 66.0f;
     Settings.ZNear = 0.1f;
     Settings.ZFar = 1000.0f;

     ApplySettings();
}

FCameraData CPerspectiveCamera::GetCameraData() const
{
    return FCameraData { GetViewMatrix(), GetProjectionMatrix() };
}

void CPerspectiveCamera::SetOrthographicProjection(float Zoom, float AspectRatio, float ZNear, float ZFar)
{
    Projection = glm::ortho(-AspectRatio * Zoom, AspectRatio * Zoom, -1.0f * Zoom, 1.0f * Zoom, ZNear, ZFar);
    Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
    Projection[3][1] *= -1.0f; // Invert Y-Axis Translation

    Settings.AspectRatio = AspectRatio;
}

void CPerspectiveCamera::SetPerspectiveProjection(float FovY, float AspectRatio, float ZNear, float ZFar)
{
    Projection = glm::perspective(FovY, AspectRatio, ZNear, ZFar);
    Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling

    Settings.AspectRatio = AspectRatio;
}

void CPerspectiveCamera::CalculateViewMatrix(const glm::vec3& NewPosition, const glm::vec3& NewRotation)
{
    glm::vec3 Forward;
    Forward.x = cos(NewRotation.x) * cos(NewRotation.y);
    Forward.y = sin(NewRotation.x);
    Forward.z = cos(NewRotation.x) * sin(NewRotation.y);
    const glm::vec3 Direction = glm::normalize(Forward);
    const glm::vec3 Right = glm::normalize(glm::cross(Direction, glm::vec3(0.0f, 1.0f, 0.0f)));
    const glm::vec3 Up    = glm::cross(Right, Direction);

    ViewMatrix = glm::lookAt(NewPosition, NewPosition + Direction, Up);
    Position = NewPosition;
    Rotation = NewRotation;
}

void CPerspectiveCamera::ApplySettings()
{
    switch (Settings.ProjectionMode)
    {
        case ECameraProjectionMode::Orthographic: SetOrthographicProjection(glm::radians(Settings.FoVY), Settings.AspectRatio, Settings.ZNear, Settings.ZFar); break;
        case ECameraProjectionMode::Perspective: SetPerspectiveProjection(glm::radians(Settings.FoVY), Settings.AspectRatio, Settings.ZNear, Settings.ZFar); break;
    }
}

void CPerspectiveCamera::UpdateCameraData(const glm::vec3& Position, const glm::vec3& Rotation)
{
    CalculateViewMatrix(Position, Rotation);
}
