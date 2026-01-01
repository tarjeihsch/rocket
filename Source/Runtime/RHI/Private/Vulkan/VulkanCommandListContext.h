#pragma once

#include "VulkanCommandBufferContext.h"
#include "Common/RHICommandList.h"

struct RHITextureAccessState;
struct RHIBufferAccessState;
struct RHIPipeline;
struct FMeshDrawCommand;

class VulkanCommandListContext final : public RHICommandListContext, public VulkanCommandBufferContext
{
public:
    explicit VulkanCommandListContext(VulkanDevice& in_device);

    void rhi_bind_pipeline(RHIPipeline *pipeline) override;
    void rhi_set_scissor_rect(Vector2i position, Vector2i extent) override;
    void rhi_pipeline_barrier_texture(RHITexture *texture, const RHITextureAccessState& from_texture_access_state, const RHITextureAccessState& to_texture_access_state) override;
    void rhi_pipeline_barrier_buffer(RHIBuffer *buffer, const RHIBufferAccessState& from_buffer_access_state, const RHIBufferAccessState& to_buffer_access_state) override;
    void rhi_copy_buffer(RHIBuffer* src, RHIBuffer* dst, uint32_t src_offset, uint32_t dst_offset, uint32_t size) override;
    void rhi_copy_image(RHITexture* src, RHITexture* dst) override;
    void rhi_begin_rendering(std::span<RHITextureView*> color_attachment_view, RHITextureView* depth_stencil_attachment, bool bClearColor) override;
    void rhi_end_rendering() override;
};