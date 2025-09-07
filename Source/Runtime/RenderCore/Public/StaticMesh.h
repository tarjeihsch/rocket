#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "Memory.h"
#include "Object.h"
#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"
#include "glm/gtx/quaternion.hpp"

struct FRHIBuffer;

class FStaticMesh : public Object
{
public:
    FStaticMesh(const char* InFilePath);
    ~FStaticMesh();

    inline FRHIBuffer* GetVertexBuffer() const
    {
        return VertexBuffer;
    }

    inline FRHIBuffer* GetIndexBuffer() const
    {
        return IndexBuffer;
    }

    inline uint32_t GetVertexCount() const
    {
        return VertexCount;
    }

    inline uint32_t GetIndexCount() const
    {
        return IndexCount;
    }

private:
    void Init();
    void Shutdown();

private:
    FRHIBuffer* VertexBuffer;
    FRHIBuffer* IndexBuffer;

    uint32_t VertexCount;
    uint32_t IndexCount;

    const char* FilePath;
};

struct FStaticMeshComponent
{
    FStaticMesh* Handle;
};

struct FTransformComponent
{
    FTransformComponent()
    {
        Translation = glm::vec3(0.0f);
        Rotation = glm::vec3(0.0f);
        Scale = glm::vec3(1.0f);
    }

    [[nodiscard]] glm::mat4 ToMatrix() const
    {
        glm::mat4 TranslationMatrix = glm::translate(glm::mat4(1.0f), Translation);
        glm::mat4 RotationMatrix = glm::toMat4(glm::quat(glm::radians(Rotation)));
        glm::mat4 ScaleMatrix = glm::scale(glm::mat4(1.0f), Scale);
        return TranslationMatrix * RotationMatrix * ScaleMatrix;
    }

    glm::vec3 Translation;
    glm::vec3 Rotation;
    glm::vec3 Scale;
};
