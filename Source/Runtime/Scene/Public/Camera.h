#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class ECameraProjectionMode 
{ 
    Perspective, Orthographic
};

struct FCameraData
{
    alignas(16) glm::mat4 ViewMatrix = glm::identity<glm::mat4>();
    alignas(16) glm::mat4 Projection = glm::identity<glm::mat4>();
};

class ICamera
{
public:
    virtual ~ICamera() = default;

    virtual FCameraData GetCameraData() const = 0;

    virtual void UpdateCameraData(const glm::vec3& Position, const glm::vec3& Rotation) = 0;
};

class CPerspectiveCamera : public ICamera
{
public:
    CPerspectiveCamera();

    virtual FCameraData GetCameraData() const override;

    void SetOrthographicProjection(float Zoom, float AspectRatio, float ZNear, float ZFar);
    void SetPerspectiveProjection(float FovY, float AspectRatio, float ZNear, float ZFar);
    void CalculateViewMatrix(const glm::vec3& Position, const glm::vec3& Rotation);
    void ApplySettings();

    virtual void UpdateCameraData(const glm::vec3& Position, const glm::vec3& Rotation) override;

    inline const glm::mat4& GetViewMatrix() const
    {
	    return ViewMatrix;
    }

    inline const glm::mat4& GetProjectionMatrix() const
    {
	    return Projection;
    }

    inline const glm::vec3& GetPosition() const
    {
	    return Position;
    }

    inline const glm::vec3& GetRotation() const
    {
	    return Rotation;
    }

    struct SCameraSettings
    {
	    ECameraProjectionMode ProjectionMode;
	    float FoVY;
	    float ZNear;
	    float ZFar;
	    float AspectRatio;
    } Settings;

private:
    glm::mat4 Projection = glm::identity<glm::mat4>();
    glm::mat4 ViewMatrix = glm::identity<glm::mat4>();

    glm::vec3 Position;
    glm::vec3 Rotation;
};
