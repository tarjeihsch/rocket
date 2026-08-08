#include "GraphicsUtils.hpp"

#include <cassert>

#include "GeometryTypes.hpp"
#include "GraphicsTypes.hpp"

#include <Math/Vector.h>

PipelineBuilder::PipelineBuilder()
{
    graphics_pipeline_desc.topology = RHI::GPUPrimitiveTopology::TriangleList;
    graphics_pipeline_desc.cull_mode = RHI::GPUCullMode::Back;
    graphics_pipeline_desc.front_face = RHI::GPUFrontFace::Clockwise;
    graphics_pipeline_desc.polygon_mode = RHI::GPUPolygonMode::Fill;
    graphics_pipeline_desc.depth_test_enable = true;
    graphics_pipeline_desc.depth_write_enable = true;
    graphics_pipeline_desc.depth_compare_op = RHI::GPUCompareOp::Greater;
    graphics_pipeline_desc.depth_format = RHI::GPUFormat::D32_FLOAT;
    graphics_pipeline_desc.blend_enable = false;
    graphics_pipeline_desc.src_blend_factor_color = RHI::GPUBlendFactor::One;
    graphics_pipeline_desc.dst_blend_factor_color = RHI::GPUBlendFactor::Zero;
    graphics_pipeline_desc.src_blend_factor_alpha = RHI::GPUBlendFactor::One;
    graphics_pipeline_desc.dst_blend_factor_alpha = RHI::GPUBlendFactor::Zero;
}

PipelineBuilder& PipelineBuilder::SetVertexShader(const u8* data, u32 size)
{
    vertex_shader_data.assign(data, data + size);
    return *this;
}

PipelineBuilder& PipelineBuilder::SetFragmentShader(const u8* data, u32 size)
{
    fragment_shader_data.assign(data, data + size);
    return *this;
}

PipelineBuilder& PipelineBuilder::SetComputeShader(const u8* data, u32 size)
{
    compute_shader_data.assign(data, data + size);
    return *this;
}

PipelineBuilder& PipelineBuilder::SetTriangleStrip()
{
    graphics_pipeline_desc.topology = RHI::GPUPrimitiveTopology::TriangleStrip;
    graphics_pipeline_desc.polygon_mode = RHI::GPUPolygonMode::Fill;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetTriangleList()
{
    graphics_pipeline_desc.topology = RHI::GPUPrimitiveTopology::TriangleList;
    graphics_pipeline_desc.polygon_mode = RHI::GPUPolygonMode::Fill;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetWireframeLine()
{
    graphics_pipeline_desc.topology = RHI::GPUPrimitiveTopology::LineList;
    graphics_pipeline_desc.polygon_mode = RHI::GPUPolygonMode::Line;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetWireframeList()
{
    graphics_pipeline_desc.topology = RHI::GPUPrimitiveTopology::TriangleList;
    graphics_pipeline_desc.polygon_mode = RHI::GPUPolygonMode::Line;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetCullMode(RHI::GPUCullMode mode)
{
    graphics_pipeline_desc.cull_mode = mode;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetFrontFace(RHI::GPUFrontFace face)
{
    graphics_pipeline_desc.front_face = face;
    return *this;
}

PipelineBuilder &PipelineBuilder::SetDepthMode(bool enable_test, bool enable_write, RHI::GPUCompareOp compare_op, RHI::GPUFormat format)
{
    graphics_pipeline_desc.depth_test_enable = enable_test;
    graphics_pipeline_desc.depth_write_enable = enable_write;
    graphics_pipeline_desc.depth_compare_op = compare_op;
    graphics_pipeline_desc.depth_format = format;
    return *this;
}

PipelineBuilder& PipelineBuilder::SetBlendMode(bool enable, RHI::GPUBlendFactor src_color, RHI::GPUBlendFactor dst_color, RHI::GPUBlendFactor srcAlpha, RHI::GPUBlendFactor dstAlpha)
{
    graphics_pipeline_desc.blend_enable = enable;
    graphics_pipeline_desc.src_blend_factor_color = src_color;
    graphics_pipeline_desc.dst_blend_factor_color = dst_color;
    graphics_pipeline_desc.src_blend_factor_alpha = srcAlpha;
    graphics_pipeline_desc.dst_blend_factor_alpha = dstAlpha;
    return *this;
}

PipelineBuilder& PipelineBuilder::AddColorFormat(RHI::GPUFormat format)
{
    graphics_pipeline_desc.color_formats[graphics_pipeline_desc.color_format_count++] = format;
    return *this;
}

GraphicsPipeline* PipelineBuilder::BuildGraphicsPipeline()
{
    assert(!vertex_shader_data.empty());
    assert(!fragment_shader_data.empty());

    GraphicsPipeline* pipeline = new GraphicsPipeline();

    RHI::GPUShaderDesc vertex_shader_desc;
    vertex_shader_desc.bytecode = vertex_shader_data.data();
    vertex_shader_desc.bytecode_size = static_cast<u32>(vertex_shader_data.size());
    vertex_shader_desc.stage_flag = RHI::GPUShaderStageFlag::Vertex;
    pipeline->vertex_shader.reset(RHI::GetRHI()->CreateShader(vertex_shader_desc));

    RHI::GPUShaderDesc fragment_shader_desc;
    fragment_shader_desc.bytecode = fragment_shader_data.data();
    fragment_shader_desc.bytecode_size = static_cast<u32>(fragment_shader_data.size());
    fragment_shader_desc.stage_flag = RHI::GPUShaderStageFlag::Fragment;
    pipeline->fragment_shader.reset(RHI::GetRHI()->CreateShader(fragment_shader_desc));

    graphics_pipeline_desc.vertex_shader = pipeline->vertex_shader.get();
    graphics_pipeline_desc.fragment_shader = pipeline->fragment_shader.get();

    pipeline->pipeline_handle.reset(RHI::GetRHI()->CreateGraphicsPipeline(graphics_pipeline_desc));

    return pipeline;
}

ComputePipeline* PipelineBuilder::BuildComputePipeline()
{
    assert(!compute_shader_data.empty());

    ComputePipeline* pipeline = new ComputePipeline();

    RHI::GPUShaderDesc compute_shader_desc;
    compute_shader_desc.bytecode = compute_shader_data.data();
    compute_shader_desc.bytecode_size = static_cast<u32>(compute_shader_data.size());
    compute_shader_desc.stage_flag = RHI::GPUShaderStageFlag::Compute;
    pipeline->fragment_shader.reset(RHI::GetRHI()->CreateShader(compute_shader_desc));

    compute_pipeline_desc.computeShader = pipeline->fragment_shader.get();
    pipeline->pipeline_handle.reset(RHI::GetRHI()->CreateComputePipeline(compute_pipeline_desc));

    return pipeline;
}

TextBuilder::TextBuilder()
    : font(nullptr)
    , scale(16.0f)
{
}

TextBuilder &TextBuilder::SetFont(const Font *font)
{
    this->font = font;
    return *this;
}

TextBuilder &TextBuilder::SetColor(Color32 color)
{
    this->color = color;
    return *this;
}

TextBuilder& TextBuilder::SetScale(float font_size)
{
    scale = font_size;
    return *this;
}

TextBuilder& TextBuilder::SetText(const std::string& msg)
{
    text = msg;
    return *this;
}

Mesh TextBuilder::Build() const
{
    assert(font != nullptr);
    
    Mesh result;

    float cursor_x = 0;
    float cursor_y = 0;

    result.aabb.min = Vector3(0, 0, 0);
    result.aabb.max = Vector3(0, 0, 0);
    result.vertices.reserve(text.size() * 6);

    bool first = true;

    for (const char c : text)
    {
        u32 unicode = static_cast<u32>(static_cast<u8>(c));

        if (unicode == '\n')
        {
            cursor_y -= scale;
            cursor_x = 0;
            continue;
        }
        if (unicode == '\t')
        {
            cursor_x += 4.0f * scale;
            continue;
        }
        if (unicode < 32)
        {
            continue;
        }

        const auto it = font->glyphs.find(unicode);
        if (it == font->glyphs.end())
        {
            continue;
        }

        const Glyph& g = it->second;

        const float left   = cursor_x + g.plane_bounds_left   * scale;
        const float bottom = cursor_y + g.plane_bounds_bottom  * scale;
        const float right  = cursor_x + g.plane_bounds_right   * scale;
        const float top    = cursor_y + g.plane_bounds_top     * scale;

        if (first)
        {
            result.aabb.min = Vector3(left, bottom, 0);
            result.aabb.max = Vector3(right, top, 0);
            first = false;
        }
        else
        {
            if (left   < result.aabb.min.x) result.aabb.min.x = left;
            if (bottom < result.aabb.min.y) result.aabb.min.y = bottom;
            if (right  > result.aabb.max.x) result.aabb.max.x = right;
            if (top    > result.aabb.max.y) result.aabb.max.y = top;
        }

        const float u0 = g.atlas_bounds_left / font->width;
        const float v0 = 1.0f - (g.atlas_bounds_bottom / font->height);
        const float u1 = g.atlas_bounds_right / font->width;
        const float v1 = 1.0f - (g.atlas_bounds_top / font->height);

        result.vertices.push_back({ Vector3(left,  bottom, 0), Vector3(u0, v0, 0), color });
        result.vertices.push_back({ Vector3(right, bottom, 0), Vector3(u1, v0, 0), color });
        result.vertices.push_back({ Vector3(right, top,    0), Vector3(u1, v1, 0), color });
        result.vertices.push_back({ Vector3(left,  bottom, 0), Vector3(u0, v0, 0), color });
        result.vertices.push_back({ Vector3(right, top,    0), Vector3(u1, v1, 0), color });
        result.vertices.push_back({ Vector3(left,  top,    0), Vector3(u0, v1, 0), color });

        cursor_x += g.advance * scale;
    }

    return result;
}

BufferBuilder &BufferBuilder::SetTarget(PageBuffer *page_buffer)
{
    target = page_buffer;
    
    return *this;
}

u64 BufferBuilder::Build()
{
    return WritePageBuffer(target, RHI::GetCurrentFrame(), data.Data(), data.Size(), alignof(std::max_align_t));
}
