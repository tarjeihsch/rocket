#pragma once

#include <cstdint>

#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"

struct FRHIDrawCommand
{
    FRHIBuffer* VertexBuffer;
    FRHIBuffer* IndexBuffer;

    uint32_t IndexCount;
    uint32_t VertexBufferOffset;
    uint32_t IndexBufferOffset;

    glm::vec3 Translation;
    glm::vec3 Rotation;
    glm::vec3 Scale;
};

struct FRHIDrawParams
{
    float DrawParam_1 = 0;
    float DrawParam_2 = 0;
    float DrawParam_3 = 0;
    float DrawParam_4 = 0;
    float DrawParam_5 = 0;
    float DrawParam_6 = 0;
};