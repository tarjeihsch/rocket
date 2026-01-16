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

enum class RHIFormat : uint8_t
{
    Unknown,

    BGRA8_SRGB,
    UNorm8x4,

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

enum class RHIPrimitiveTopology : uint8_t
{
    TriangleList,
    TriangleStrip,
    LineList,
    LineStrip,
    PointList,
    PatchList, // For tessellation
};

enum class RHICullMode : uint8_t
{
    None,
    Front,
    Back,
    FrontAndBack,
};

enum class RHIPolygonMode : uint8_t
{
    Fill,
    Line,
    Point,
};

struct RHIVertexBinding
{
    RHIVertexBinding(uint32_t in_binding, uint32_t in_stride, bool in_instance)
        : binding(in_binding), stride(in_stride), instance(in_instance)
    {
    }

    uint32_t binding;
    uint32_t stride;
    bool instance;
};

struct RHIVertexAttribute
{
    RHIVertexAttribute(uint32_t InLocation, uint32_t InBinding, RHIFormat InFormat, uint32_t InOffset)
        : Location(InLocation), Binding(InBinding), Format(InFormat), Offset(InOffset)
    {
    }

    uint32_t Location;
    uint32_t Binding;
    RHIFormat Format;
    uint32_t Offset;
};

enum class RHIBlendFactor : uint8_t
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Src1Color,
    OneMinusSrc1Color,
    Src1Alpha,
    OneMinusSrc1Alpha,
};

enum class RHIBlendOp : uint8_t
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

struct RHIBlendAttachment
{
    uint32_t bEnableBlend = false;

    // Color blending
    RHIBlendFactor SrcColorFactor = RHIBlendFactor::One;
    RHIBlendFactor DstColorFactor = RHIBlendFactor::Zero;
    RHIBlendOp ColorOp = RHIBlendOp::Add;

    // Alpha blending
    RHIBlendFactor SrcAlphaFactor = RHIBlendFactor::One;
    RHIBlendFactor DstAlphaFactor = RHIBlendFactor::Zero;
    RHIBlendOp AlphaOp = RHIBlendOp::Add;

    // Write mask
    uint8_t ColorWriteMask = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
};

struct RHIVertex
{
    alignas(16) glm::vec3 position;
    alignas(16) glm::vec3 normal;
    alignas(16) glm::vec2 tex_coord;
};

enum class RHIPipelineType
{
    Vertex, Pixel, Compute,
};

struct RHIPipelineDesc
{
    RHIPipelineType Type;

    std::vector<RHIFormat> color_attachment_formats;
    std::vector<RHIVertexBinding> VertexBindings;
    std::vector<RHIVertexAttribute> VertexAttributes;
    std::vector<RHIBlendAttachment> BlendAttachments;

    const char* vertex_shader_path;
    const char* pixel_shader_path;
    const char* compute_shader_path;

    RHIFormat depth_format = RHIFormat::Unknown;
    RHIFormat stencil_format = RHIFormat::Unknown;

    RHIPrimitiveTopology topology = RHIPrimitiveTopology::TriangleList;
    RHICullMode cull_mode = RHICullMode::None;
    RHIPolygonMode polygon_mode = RHIPolygonMode::Fill;

    uint8_t depth_test_enable : 1;
    uint8_t depth_write_enable : 1;
};

struct RHIPipeline
{
    virtual void* GetHandle() const = 0;
};