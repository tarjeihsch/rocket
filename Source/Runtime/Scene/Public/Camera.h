#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#pragma once

#include <optional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class ECameraProjectionMode 
{ 
    Perspective, Orthographic
};

struct FCameraCaptureResult
{
    alignas(16) glm::mat4 ViewMatrix = glm::identity<glm::mat4>();

    alignas(16) glm::mat4 Projection = glm::identity<glm::mat4>();
};

struct FCameraCaptureSettings
{
    // Position in world space
    glm::vec3 Position;

    // Rotation in euler angles
    glm::vec3 Rotation;

    // Viewing volume
    ECameraProjectionMode ProjectionMode;

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
};

class ICamera
{
public:
    // NB: Function calls to Capture should be of a minimum to reduce expensive mathematical operations
    virtual void Capture(const FCameraCaptureSettings& CaptureData) = 0;

    virtual const FCameraCaptureResult& GetCachedCapture() const = 0;
};

class CFreeLookCamera : public ICamera
{
public:
    virtual void Capture(const FCameraCaptureSettings& CaptureData) override;
    virtual const FCameraCaptureResult& GetCachedCapture() const override;

    FCameraCaptureResult CachedCaptureResult;
};