#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "Memory.h"

#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"
#include "glm/gtx/quaternion.hpp"

struct FRHIBuffer;

struct FVertex
{
    alignas(16) glm::vec3 M_Position;
    alignas(16) glm::vec3 M_Normal;
    alignas(16) glm::vec2 M_TexCoord;
};

struct FPrimitiveMeshSection
{
    uint32_t M_VertexCount;
    uint32_t M_IndexCount;

    uint32_t M_VertexOffset;
    uint32_t M_IndexOffset;

    glm::mat4 M_LocalMatrix;
};

class CStaticMesh
{
public:
    CStaticMesh(const char* InFilePath);
    ~CStaticMesh();

    inline std::span<const FPrimitiveMeshSection> GetMeshSections() const;
    inline FRHIBuffer* GetVertexBuffer() const;
    inline FRHIBuffer* GetIndexBuffer() const;
    inline uint32_t GetVertexCount() const;
    inline uint32_t GetIndexCount() const;

private:
    void Init();
    void Shutdown();

private:
    std::vector<FPrimitiveMeshSection> M_Sections;

    FRHIBuffer* M_VertexBuffer;
    FRHIBuffer* M_IndexBuffer;

    uint32_t M_VertexCount;
    uint32_t M_IndexCount;

    // TODO: Handle hot-reloading of meshes
    const char* FilePath;
};

std::span<const FPrimitiveMeshSection> CStaticMesh::GetMeshSections() const
{
    return std::span(M_Sections);
}

inline FRHIBuffer* CStaticMesh::GetVertexBuffer() const
{
    return M_VertexBuffer;
}

inline FRHIBuffer* CStaticMesh::GetIndexBuffer() const
{
    return M_IndexBuffer;
}

inline uint32_t CStaticMesh::GetVertexCount() const
{
    return M_VertexCount;
}

inline uint32_t CStaticMesh::GetIndexCount() const
{
    return M_IndexCount;
}

struct StaticMeshComponent
{
    StaticMeshComponent(CStaticMesh* P_StaticMesh)
        : Handle(P_StaticMesh)
    {
    }

    CStaticMesh* Handle;
};