#pragma once

#include <cstdint>

#include "glm/fwd.hpp"
#include "glm/detail/type_quat.hpp"

struct FRHIDrawCommand
{
    FRHIBuffer* VertexBuffer;
    FRHIBuffer* IndexBuffer;

    uint32_t IndexCount;

    glm::mat4 Transform;
};

struct FRHIDrawParams
{
    uint32_t DrawParam_1 = 0;
    uint32_t DrawParam_2 = 0;
    uint32_t DrawParam_3 = 0;
    uint32_t DrawParam_4 = 0;
};