#pragma once

#include "VulkanCommandBufferContext.h"
#include "Common/RHICommandList.h"

struct RHITextureAccessState;
struct RHIBufferAccessState;
struct RHIPipeline;
struct FMeshDrawCommand;

class VulkanCommandListContext final : public RHICommandListContext, public VulkanCommandBufferContext
{
    VulkanDevice& Device;

public:
    explicit VulkanCommandListContext(VulkanDevice& in_device);

    void RHIBindGlobalHeap() override;
    void RHIBindPipeline(RHIPipeline *pipeline) override;
    void RHISetScissorRect(Vector2i position, Vector2i extent) override;
	void RHISetViewport(Vector2i position, Vector2i extent) override;
    void RHIPipelineBarrierTexture(RHITexture *texture, const RHITextureAccessState& from_texture_access_state, const RHITextureAccessState& to_texture_access_state) override;
    void RHIPipelineBarrierBuffer(RHIBuffer *buffer, const RHIBufferAccessState& from_buffer_access_state, const RHIBufferAccessState& to_buffer_access_state) override;
    void RHICopyBuffer(RHIBuffer* src, RHIBuffer* dst, uint32_t src_offset, uint32_t dst_offset, uint32_t size) override;
    void RHICopyImage(RHITexture* src, RHITexture* dst) override;
	void RHIDraw(uint32_t VertexCount, uint32_t InstanceCount, uint32_t FirstVertex, uint32_t FirstInstance) override;
    void RHIDrawIndexed(uint32_t uint32) override;
    void RHIDrawIndirectCount(RHIBuffer *indirect_draw_command_buffer, size_t offset, RHIBuffer *count_buffer, size_t count_offset, uint32_t max_draw_count, uint32_t stride) override;
    void RHIDrawIndexedIndirectCount(RHIBuffer* indirect_draw_command_buffer, size_t offset, RHIBuffer* count_buffer, size_t count_offset, uint32_t max_draw_count, uint32_t stride) override;
    void RHIBeginRendering(const RHIRenderPassInfo& Info) override;
    void RHIEndRendering() override;
    void RHIAddWaitSemaphore(RHISemaphore* semaphore, RHIPipelineStage pipeline_stage) override;
    void RHIAddSignalSemaphore(RHISemaphore* semaphore, RHIPipelineStage pipeline_stage) override;
    void RHIBindVertexBuffer(RHIBuffer * rhi_buffer) override;
    void RHIBindIndexBuffer(RHIBuffer * rhi_buffer) override;
	void RHISetShaderParameters(const void* Data, uint32_t Size) override;
	void RHIDebugMarkTimelineBegin(const ANSICHAR* Name) override;
	void RHIDebugMarkTimelineEnd() override;
};