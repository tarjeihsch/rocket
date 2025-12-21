#pragma once

#include <array>
#include <cstdint>
#include <memory>

class RMaterial;
class FRHIBuffer;

struct FRHIDrawCommand
{
    enum class EIndexType : uint8_t { U32, U16 };

    FRHIBuffer* VertexBuffer;
    FRHIBuffer* IndexBuffer;

    uint32_t IndexCount;
    uint32_t VertexBufferOffset;
    uint32_t IndexBufferOffset;

    EIndexType IndexType;
};

struct FRHIDrawParams
{
    uint32_t DrawParam_U32_1 = 0;
    uint32_t DrawParam_U32_2 = 0;
    uint32_t DrawParam_U32_3 = 0;
    uint32_t DrawParam_U32_4 = 0;

    float DrawParam_F32_1 = 0.0f;
    float DrawParam_F32_2 = 0.0f;
    float DrawParam_F32_3 = 0.0f;
    float DrawParam_F32_4 = 0.0f;
};

class RPrimitive;

struct FRHIDrawCall
{
    enum class EIndexType : uint8_t
    {
        U32, U16
    };

    std::shared_ptr<RPrimitive> Primitive;
    std::shared_ptr<RMaterial> Material;

    FRHIDrawParams Params;

    std::array<float, 2> ScissorPosition;
    std::array<float, 2> ScissorExtent;

    EIndexType IndexType;
};
