#pragma once

#include <glm/glm.hpp>
#include <memory>

struct FRHICommandList;
struct FRHIDrawCall;
struct FRHIBuffer;

struct RVertex
{
    glm::vec2 Position;
    glm::vec2 UV;
    glm::vec4 Color;
};

using RIndex = uint16_t;

class RPrimitive
{
public:
    RPrimitive(const std::vector<RVertex>& Vertices, const std::vector<RIndex>& Indices);
    RPrimitive(FRHIBuffer* VertexBuffer, FRHIBuffer* IndexBuffer);

    FRHIBuffer* GetVB() const;
    FRHIBuffer* GetIB() const;
    int32_t GetIndexCount();

private:
    std::unique_ptr<FRHIBuffer> VertexBuffer;
    std::unique_ptr<FRHIBuffer> IndexBuffer;
};