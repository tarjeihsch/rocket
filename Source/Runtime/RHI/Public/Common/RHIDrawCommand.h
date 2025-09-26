#pragma once

#include <cstdint>

#include "glm/fwd.hpp"

struct FRHIDrawCommand
{
    enum class EIndexType : uint8_t { U32, U16 };

    FRHIBuffer* VertexBuffer;
    FRHIBuffer* IndexBuffer;

    glm::vec3 Translation;
    glm::vec3 Rotation;
    glm::vec3 Scale;

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