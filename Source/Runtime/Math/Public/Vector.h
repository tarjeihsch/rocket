#pragma once

#include <glm/glm.hpp>

namespace Math
{
    inline glm::vec3 GetForwardVector(glm::vec3 P_Rotation)
    {
        glm::vec3 ForwardVector;
        ForwardVector.x = glm::cos(P_Rotation.y) * glm::cos(P_Rotation.x);
        ForwardVector.y = glm::sin(P_Rotation.x);
        ForwardVector.z = glm::sin(P_Rotation.y) * glm::cos(P_Rotation.x);
        ForwardVector = glm::normalize(ForwardVector);
        return ForwardVector;
    }

    inline glm::vec3 GetUpVector()
    {
        return { 0.0f, 1.0f, 0.0f };
    }

    inline glm::vec3 GetRightVector(glm::vec3 P_Rotation)
    {
        return glm::normalize(glm::cross(GetForwardVector(P_Rotation), GetUpVector()));
    }
}
