#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "Memory.h"
#include "Object.h"
#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"
#include "glm/gtx/quaternion.hpp"

struct FRHIBuffer;

class CStaticMesh : public CObject
{
public:
    CStaticMesh(const char* InFilePath);
    ~CStaticMesh();

    inline FRHIBuffer* GetVertexBuffer() const;
    inline FRHIBuffer* GetIndexBuffer() const;
    inline uint32_t GetVertexCount() const;
    inline uint32_t GetIndexCount() const;

private:
    void Init();
    void Shutdown();

private:
    FRHIBuffer* M_VertexBuffer;
    FRHIBuffer* M_IndexBuffer;

    uint32_t M_VertexCount;
    uint32_t M_IndexCount;

    // TODO: Handle hot-reloading of meshes
    const char* FilePath;
};

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