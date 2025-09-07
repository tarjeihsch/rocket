#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include <cstdint>
#include <vector>

/*
 * Vulkan states each PSO being an immutable object after creation. This means that we can not alter the state of the pipeline.
 * One way to solve this issue is to go by a specification approach. We only specify the specification we expect of the pipeline,
 * and depending on the hashed result we either get an existing one or a new.
 */

struct ShaderAsset;

enum class ERHIFormat : uint8_t
{
    Unknown,

    BGRA8_SRGB,

    // Color formats
    RGBA8_UNorm,
    BGRA8_UNorm,
    RGBA16_Float,
    RGBA32_Float,

    Float,
    Float2,
    Float3,
    Float4,

    // Depth/Stencil
    D24S8,
    D32_Float,
    D32_Float_S8X24,

    // Add more as needed
};

enum class ERHIPrimitiveTopology : uint8_t
{
    TriangleList,
    TriangleStrip,
    LineList,
    LineStrip,
    PointList,
    PatchList, // For tessellation
};

enum class ERHICullMode : uint8_t
{
    None,
    Front,
    Back,
    FrontAndBack,
};

enum class ERHIPolygonMode : uint8_t
{
    Fill,
    Line,
    Point,
};

struct FRHIVertexBinding
{
    uint32_t Binding = 0;
    uint32_t Stride = 0;
    uint8_t bPerInstance : 1;
};

struct FRHIVertexAttribute
{
    uint32_t Location = 0;
    uint32_t Binding = 0;
    ERHIFormat Format = ERHIFormat::Unknown;
    uint32_t Offset = 0;
};

struct FRHIVertex
{
    alignas(16) glm::vec3 m_Position;
    alignas(16) glm::vec3 m_Normal;
    alignas(16) glm::vec2 m_TexCoord;
};

struct FRHIPipelineDesc
{
    ShaderAsset* VertexShader;
    ShaderAsset* PixelShader;

    std::vector<ERHIFormat> ColorAttachmentFormats;
    ERHIFormat DepthFormat = ERHIFormat::Unknown;
    ERHIFormat StencilFormat = ERHIFormat::Unknown;

    std::vector<FRHIVertexBinding> VertexBindings;
    std::vector<FRHIVertexAttribute> VertexAttributes;

    ERHIPrimitiveTopology Topology = ERHIPrimitiveTopology::TriangleList;
    ERHICullMode CullMode = ERHICullMode::None;
    ERHIPolygonMode PolygonMode = ERHIPolygonMode::Fill;

    uint8_t bDepthTestEnable : 1;
    uint8_t bDepthWriteEnable : 1;
};

struct FRHIPipeline
{
    virtual void* GetHandle() const = 0;
};