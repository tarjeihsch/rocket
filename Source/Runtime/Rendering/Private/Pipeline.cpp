#include "Pipeline.h"

#include <fstream>
#include <iosfwd>
#include <vector>

#include "Common/RHI.h"
#include "Common/RHIPipeline.h"

GraphicsPipeline::GraphicsPipeline(const ANSICHAR *vertex_shader_ir_path, const ANSICHAR *pixel_shader_ir_path)
{
    RHIPipelineDesc pipeline_desc = {};
    pipeline_desc.vertex_shader_path = vertex_shader_ir_path;
    pipeline_desc.pixel_shader_path = pixel_shader_ir_path;
    pipeline_desc.compute_shader_path = nullptr;
    pipeline_desc.color_attachment_formats.push_back(RHIFormat::BGRA8_SRGB);
    pipeline_desc.depth_test_enable = true;
    pipeline_desc.depth_write_enable = true;
    pipeline_desc.depth_format = RHIFormat::D32_Float_S8X24;
    pipeline_desc.stencil_format = RHIFormat::Unknown;
    pipeline_desc.cull_mode = RHICullMode::Back;
    pipeline_desc.polygon_mode = RHIPolygonMode::Fill;
    pipeline_desc.topology = RHIPrimitiveTopology::TriangleList;
    pipeline_desc.VertexBindings.emplace_back(0, static_cast<uint32_t>(sizeof(RHIVertex)), false);
    pipeline_desc.VertexAttributes.emplace_back(0, 0, RHIFormat::Float3, static_cast<uint32_t>(offsetof(RHIVertex, position)));
    pipeline_desc.VertexAttributes.emplace_back(1, 0, RHIFormat::Float3, static_cast<uint32_t>(offsetof(RHIVertex, normal)));
    pipeline_desc.VertexAttributes.emplace_back(2, 0, RHIFormat::Float2, static_cast<uint32_t>(offsetof(RHIVertex, tex_coord)));

    RHIBlendAttachment blend_attachment = {};
    blend_attachment.bEnableBlend = true;
    blend_attachment.SrcColorFactor = RHIBlendFactor::SrcAlpha;
    blend_attachment.DstColorFactor = RHIBlendFactor::OneMinusSrcAlpha;
    blend_attachment.ColorOp        = RHIBlendOp::Add;
    blend_attachment.SrcAlphaFactor = RHIBlendFactor::One;
    blend_attachment.DstAlphaFactor = RHIBlendFactor::OneMinusSrcAlpha;
    blend_attachment.AlphaOp        = RHIBlendOp::Add;
    pipeline_desc.BlendAttachments.push_back(blend_attachment);

    rhi_pipeline = GetRHI()->RHICreatePipeline(pipeline_desc);
}

ComputePipeline::ComputePipeline(const ANSICHAR* compute_shader_ir_path)
{
    RHIPipelineDesc pipeline_desc = {};
    pipeline_desc.compute_shader_path = compute_shader_ir_path;

    rhi_pipeline = GetRHI()->RHICreatePipeline(pipeline_desc);
}