#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/fwd.hpp"
#include "glm/gtx/quaternion.hpp"

#include "Primitive.h"

struct FRHIBuffer;

struct FVertex
{
    alignas(16) glm::vec3 M_Position;
    alignas(16) glm::vec3 M_Normal;
    alignas(16) glm::vec2 M_TexCoord;
};

struct FPrimitiveMeshSection
{
    uint32_t M_VertexCount = 0;
    uint32_t M_IndexCount = 0;

    uint32_t M_VertexOffset = 0;
    uint32_t M_IndexOffset = 0;

    glm::mat4 LocalMatrix;
};

class RStaticMesh : public RPrimitive
{
public:
    RStaticMesh(const char* InFilePath);
    ~RStaticMesh();

    virtual void Draw(FRHICommandList& CmdList) const override;

    inline FRHIBuffer* GetVertexBuffer() const;
    inline FRHIBuffer* GetIndexBuffer() const;
    inline uint32_t GetVertexCount() const;
    inline uint32_t GetIndexCount() const;

private:
    void Init();
    void Shutdown();

private:
    std::vector<FPrimitiveMeshSection> Sections;

    FRHIBuffer* M_VertexBuffer;
    FRHIBuffer* M_IndexBuffer;

    uint32_t M_VertexCount;
    uint32_t M_IndexCount;

    // TODO: Handle hot-reloading of meshes
    const char* FilePath;
};

inline FRHIBuffer* RStaticMesh::GetVertexBuffer() const
{
    return M_VertexBuffer;
}

inline FRHIBuffer* RStaticMesh::GetIndexBuffer() const
{
    return M_IndexBuffer;
}

inline uint32_t RStaticMesh::GetVertexCount() const
{
    return M_VertexCount;
}

inline uint32_t RStaticMesh::GetIndexCount() const
{
    return M_IndexCount;
}
