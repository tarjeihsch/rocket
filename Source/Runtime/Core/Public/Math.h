#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct FTransform
{
    FTransform()
    {
        M_Translation = glm::vec3(0.0f);
        M_Rotation = glm::vec3(0.0f);
        M_Scale = glm::vec3(1.0f);
    }

    [[nodiscard]] glm::mat4 ToMatrix() const
    {
        glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), M_Translation);
        glm::mat4 RotationMatrix = glm::toMat4(glm::quat(glm::radians(M_Rotation)));
        glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), M_Scale);
        return TranslationMatrix * RotationMatrix * ScaleMatrix;
    }

    glm::vec3 M_Translation;
    glm::vec3 M_Rotation;
    glm::vec3 M_Scale;
};

struct TransformComponent
{
    TransformComponent() = default;
    TransformComponent(FTransform P_Transform)
    {
        M_Transform = P_Transform;
    }

    FTransform M_Transform;
};