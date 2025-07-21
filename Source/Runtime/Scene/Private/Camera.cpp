#include "Camera.h"

void PCamera::SetOrthographicProjection(float Zoom, float AspectRatio, float ZNear, float ZFar)
{
    Projection = glm::ortho(-AspectRatio * Zoom, AspectRatio * Zoom, -1.0f * Zoom, 1.0f * Zoom, ZNear, ZFar);
    Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling
    Projection[3][1] *= -1.0f; // Invert Y-Axis Translation

    Settings.AspectRatio = AspectRatio;
}

void PCamera::SetPerspectiveProjection(float FovY, float AspectRatio, float ZNear, float ZFar)
{
    Projection = glm::perspective(FovY, AspectRatio, ZNear, ZFar);
    Projection[1][1] *= -1.0f; // Invert Y-Axis Screen Space Scaling

    Settings.AspectRatio = AspectRatio;
}

void PCamera::CalculateViewMatrix(const glm::vec3& NewPosition, const glm::vec3& NewRotation)
{
    const glm::vec3 Forward = glm::vec3(glm::sin(NewRotation.y) * glm::cos(NewRotation.x), glm::cos(NewRotation.y) * glm::cos(NewRotation.x), glm::sin(NewRotation.x));
    const glm::vec3 Direction = glm::normalize(Forward);
    const glm::vec3 Right = glm::normalize(glm::cross(Direction, glm::vec3(0.0f, 0.0f, 1.0f)));
    const glm::vec3 Up = glm::cross(Right, Direction);

    ViewMatrix = glm::lookAt(NewPosition, NewPosition + Direction, Up);
    Position = NewPosition;
    Rotation = NewRotation;
}

void PCamera::ApplySettings()
{
    switch (Settings.ProjectionMode)
    {
        case ECameraProjectionMode::Orthographic: SetOrthographicProjection(glm::radians(Settings.FoVY), Settings.AspectRatio, Settings.ZNear, Settings.ZFar); break;
        case ECameraProjectionMode::Perspective: SetPerspectiveProjection(glm::radians(Settings.FoVY), Settings.AspectRatio, Settings.ZNear, Settings.ZFar); break;
    }
}
