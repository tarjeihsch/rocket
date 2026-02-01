#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "InlineArray.h"
#include "RHIBuffer.h"
#include "RHITexture.h"
#include "Math/Vector2.h"
#include "Common/RHICommon.h"
#include "RHITextureView.h"

enum class RHISyncStage : uint8_t;
class RHISwapchain;
struct RHISemaphore;
struct RHIBuffer;
struct RHIPipeline;
struct RHITexture;
struct RHITextureView;

class RHICommandListContext
{
public:
    virtual ~RHICommandListContext() = default;

    virtual RHICommandListContext& get_lowest_level_context() { return *this; }

    virtual void RHIBindGlobalHeap() = 0;
    virtual void RHIBindPipeline(RHIPipeline *pipeline) = 0;
	virtual void RHISetScissorRect(Vector2i position, Vector2i extent) = 0;
	virtual void RHISetViewport(Vector2i position, Vector2i extent) = 0;
    virtual void RHIPipelineBarrierTexture(RHITexture *texture, const RHITextureAccessState& from_texture_access_state, const RHITextureAccessState& to_texture_access_state) = 0;
    virtual void RHIPipelineBarrierBuffer(RHIBuffer *buffer, const RHIBufferAccessState& from_buffer_access_state, const RHIBufferAccessState& to_buffer_access_state) = 0;
    virtual void RHICopyBuffer(RHIBuffer* src, RHIBuffer* dst, uint32_t src_offset, uint32_t dst_offset, uint32_t size) = 0;
    virtual void RHICopyImage(RHITexture* src, RHITexture* dst) = 0;
    virtual void RHIDraw(uint32_t VertexCount, uint32_t InstanceCount, uint32_t FirstVertex, uint32_t FirstInstance) = 0;
    virtual void RHIDrawIndexed(uint32_t uint32) = 0;
    virtual void RHIDrawIndirectCount(RHIBuffer *indirect_draw_command_buffer, size_t offset, RHIBuffer *count_buffer, size_t count_offset, uint32_t max_draw_count, uint32_t stride) = 0;
    virtual void RHIDrawIndexedIndirectCount(RHIBuffer* indirect_draw_command_buffer, size_t offset, RHIBuffer* count_buffer, size_t count_offset, uint32_t max_draw_count, uint32_t stride) = 0;
    virtual void RHIBeginRendering(const RHIRenderPassInfo& Info) = 0;
    virtual void RHIEndRendering() = 0;
    virtual void RHIAddWaitSemaphore(RHISemaphore*, RHIPipelineStage) = 0;
    virtual void RHIAddSignalSemaphore(RHISemaphore*, RHIPipelineStage) = 0;
    virtual void RHIBindVertexBuffer(RHIBuffer * rhi_buffer) = 0;
    virtual void RHIBindIndexBuffer(RHIBuffer * rhi_buffer) = 0;
    virtual void RHISetShaderParameters(const void* Data, uint32_t Size) = 0;
    virtual void RHIDebugMarkTimelineBegin(const ANSICHAR* Name) = 0;
    virtual void RHIDebugMarkTimelineEnd() = 0;
};

struct RHICommandBase
{
    using func = void(*)(RHICommandBase*, RHICommandListContext&);

    func execute_and_destruct = nullptr;
    uint32_t byte_size = 0;

    void Dispatch(RHICommandListContext& cmd_list_ctx)
    {
        execute_and_destruct(this, cmd_list_ctx);
    }
};

template<typename T>
struct RHICommand : RHICommandBase
{
    RHICommand()
    {
        byte_size = sizeof(T);
        execute_and_destruct = [](RHICommandBase* command_base, RHICommandListContext& cmd_list_ctx)
        {
            auto* Self = static_cast<T*>(command_base);
            Self->execute_internal(cmd_list_ctx);
            Self->~T();
        };
    }
};

class RHICommandList
{
public:
    template<typename Command, typename... Args>
    void Enqueue(Args&&... args)
    {
        static_assert(std::is_base_of_v<RHICommandBase, Command>);
        size_t Offset = buffer.size();
        buffer.resize(Offset + sizeof(Command));
        new (buffer.data() + Offset) Command(std::forward<Args>(args)...);
    }

    std::vector<uint8_t> Flush()
    {
        std::vector<uint8_t> Temp;
        Temp.swap(buffer);
        return Temp;
    }

private:
    std::vector<uint8_t> buffer;
};

class RHICommandListImmediate : public RHICommandList
{
public:
    explicit RHICommandListImmediate(RHICommandListContext& Ctx)
        : cmd_list_ctx(Ctx)
    {
    }

    template<class TCommand, class... TArgs>
    void EnqueueImmediate(TArgs&&... Args)
    {
        TCommand Command(std::forward<TArgs>(Args)...);
        Command.execute_internal(cmd_list_ctx);
    }

private:
    RHICommandListContext& cmd_list_ctx;
};

class RHICommandListExecutor
{
public:
    static void Execute(RHICommandListContext& cmd_list_ctx, std::vector<uint8_t>&& buffer)
    {
        uint8_t* start_pointer = buffer.data();
        uint8_t* end_pointer = start_pointer + buffer.size();

        while (start_pointer < end_pointer)
        {
            auto* Command = reinterpret_cast<RHICommandBase*>(start_pointer);
            Command->Dispatch(cmd_list_ctx);
            start_pointer += Command->byte_size;
        }
    }
};

struct RHICommandBindGlobalHeap : RHICommand<RHICommandBindGlobalHeap>
{
    RHICommandBindGlobalHeap()
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIBindGlobalHeap();
    }
};

struct RHICommandPipelineBarrierTexture : RHICommand<RHICommandPipelineBarrierTexture>
{
    RHITexture* texture;
    RHITextureAccessState from_texture_access_state;
    RHITextureAccessState to_texture_access_state;

    RHICommandPipelineBarrierTexture(RHITexture* in_texture, const RHITextureAccessState& in_from_texture_access_state, const RHITextureAccessState& in_to_texture_access_state)
        : texture(in_texture),
        from_texture_access_state(in_from_texture_access_state),
        to_texture_access_state(in_to_texture_access_state)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIPipelineBarrierTexture(texture, from_texture_access_state, to_texture_access_state);
    }
};

struct RHICommandPipelineBarrierBuffer : RHICommand<RHICommandPipelineBarrierBuffer>
{
    RHIBuffer* buffer;
    RHIBufferAccessState from_buffer_access_state;
    RHIBufferAccessState to_buffer_access_state;

    RHICommandPipelineBarrierBuffer(RHIBuffer* in_buffer, const RHIBufferAccessState& in_from_buffer_access_state, const RHIBufferAccessState& in_to_buffer_access_state)
        : buffer(in_buffer),
        from_buffer_access_state(in_from_buffer_access_state),
        to_buffer_access_state(in_to_buffer_access_state)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIPipelineBarrierBuffer(buffer, from_buffer_access_state, to_buffer_access_state);
    }
};

struct RHICommandBeginRendering : RHICommand<RHICommandBeginRendering>
{
    RHIRenderPassInfo Info;

    RHICommandBeginRendering(const RHIRenderPassInfo& InInfo)
        : Info(InInfo)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIBeginRendering(Info);
    }
};

struct RHICommandEndRendering : RHICommand<RHICommandEndRendering>
{
    RHICommandEndRendering()
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIEndRendering();
    }
};

struct RHICommandDraw : RHICommand<RHICommandDraw>
{
	uint32_t vertexCount;
	uint32_t instanceCount;
	uint32_t firstVertex;
    uint32_t firstInstance;

	explicit RHICommandDraw(uint32_t in_vertexCount, uint32_t in_instanceCount, uint32_t in_firstVertex, uint32_t in_firstInstance)
		: vertexCount(in_vertexCount), instanceCount(in_instanceCount), firstVertex(in_firstVertex), firstInstance(in_firstInstance)
	{
	}

	void execute_internal(RHICommandListContext& cmd_list_ctx)
	{
		cmd_list_ctx.RHIDraw(vertexCount, instanceCount, firstVertex, firstInstance);
	}
};

struct RHICommandBindPipeline : RHICommand<RHICommandBindPipeline>
{
    RHIPipeline* pipeline;

    explicit RHICommandBindPipeline(RHIPipeline* in_pipeline)
        : pipeline(in_pipeline)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIBindPipeline(pipeline);
    }
};

struct RHICommandCopyBuffer : RHICommand<RHICommandCopyBuffer>
{
    RHIBuffer* src_buffer;
    RHIBuffer* dst_buffer;
    uint32_t src_offset;
    uint32_t dst_offset;
    uint32_t size;

    RHICommandCopyBuffer(RHIBuffer* in_src_buffer, RHIBuffer* in_dst_buffer, uint32_t in_src_offset, uint32_t in_dst_offset, uint32_t in_size)
        : src_buffer(in_src_buffer), dst_buffer(in_dst_buffer), src_offset(in_src_offset), dst_offset(in_dst_offset), size(in_size)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHICopyBuffer(src_buffer, dst_buffer, src_offset, dst_offset, size);
    }
};

struct RHICommandCopyImage : RHICommand<RHICommandCopyImage>
{
    RHITexture *src;
    RHITexture *dst;

    RHICommandCopyImage(RHITexture* in_src, RHITexture* in_dst)
        : src(in_src), dst(in_dst)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHICopyImage(src, dst);
    }
};

struct RHICommandSetScissor : RHICommand<RHICommandSetScissor>
{
    Vector2i position;
    Vector2i extent;

    RHICommandSetScissor(Vector2i in_position, Vector2i in_extent)
        : position(in_position), extent(in_extent)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHISetScissorRect(position, extent);
    }
};

struct RHICommandSetViewport : RHICommand<RHICommandSetViewport>
{
	Vector2i position;
	Vector2i extent;

    RHICommandSetViewport(Vector2i in_position, Vector2i in_extent)
		: position(in_position), extent(in_extent)
	{
	}

	void execute_internal(RHICommandListContext& cmd_list_ctx)
	{
		cmd_list_ctx.RHISetViewport(position, extent);
	}
};

struct RHICommandBindVertexBuffer : RHICommand<RHICommandBindVertexBuffer>
{
    RHIBuffer* vertex_buffer;

    RHICommandBindVertexBuffer(RHIBuffer* in_vertex_buffer)
        : vertex_buffer(in_vertex_buffer)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIBindVertexBuffer(vertex_buffer);
    }
};

struct RHICommandBindIndexBuffer : RHICommand<RHICommandBindVertexBuffer>
{
    RHIBuffer* index_buffer;

    RHICommandBindIndexBuffer(RHIBuffer* in_index_buffer)
        : index_buffer(in_index_buffer)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIBindIndexBuffer(index_buffer);
    }
};

struct RHICommandDrawIndexed : RHICommand<RHICommandDrawIndexed>
{
    uint32_t index_count;

    RHICommandDrawIndexed(uint32_t in_index_count)
        : index_count(in_index_count)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIDrawIndexed(index_count);
    }
};

struct RHICommandDrawIndirectCount : RHICommand<RHICommandDrawIndirectCount>
{
    RHIBuffer* indirect_argument_buffer;
    size_t indirect_argument_buffer_offset;
    RHIBuffer* indirect_count_buffer;
    size_t indirect_count_buffer_offset;
    uint32_t max_draw_indirect_count;
    uint32_t stride;

    RHICommandDrawIndirectCount(RHIBuffer* in_indirect_argument_buffer,
        size_t in_indirect_argument_buffer_offset,
        RHIBuffer* in_indirect_count_buffer,
        size_t in_indirect_count_buffer_offset,
        uint32_t in_max_draw_indirect_count,
        uint32_t in_stride)
            : indirect_argument_buffer(in_indirect_argument_buffer),
        indirect_argument_buffer_offset(in_indirect_argument_buffer_offset),
        indirect_count_buffer(in_indirect_count_buffer),
        indirect_count_buffer_offset(in_indirect_count_buffer_offset),
        max_draw_indirect_count(in_max_draw_indirect_count),
        stride(in_stride)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIDrawIndirectCount(indirect_argument_buffer, indirect_argument_buffer_offset, indirect_count_buffer, indirect_count_buffer_offset, max_draw_indirect_count, stride);
    }
};

struct RHICommandDrawIndexedIndirectCount : RHICommand<RHICommandDrawIndexedIndirectCount>
{
    RHIBuffer* indirect_argument_buffer;
    size_t indirect_argument_buffer_offset;
    RHIBuffer* indirect_count_buffer;
    size_t indirect_count_buffer_offset;
    uint32_t max_draw_indirect_count;
    uint32_t stride;

    RHICommandDrawIndexedIndirectCount(RHIBuffer* in_indirect_argument_buffer,
        size_t in_indirect_argument_buffer_offset,
        RHIBuffer* in_indirect_count_buffer,
        size_t in_indirect_count_buffer_offset,
        uint32_t in_max_draw_indirect_count,
        uint32_t in_stride)
            : indirect_argument_buffer(in_indirect_argument_buffer),
        indirect_argument_buffer_offset(in_indirect_argument_buffer_offset),
        indirect_count_buffer(in_indirect_count_buffer),
        indirect_count_buffer_offset(in_indirect_count_buffer_offset),
        max_draw_indirect_count(in_max_draw_indirect_count),
        stride(in_stride)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIDrawIndexedIndirectCount(indirect_argument_buffer, indirect_argument_buffer_offset, indirect_count_buffer, indirect_count_buffer_offset, max_draw_indirect_count, stride);
    }
};

struct RHICommandAddWaitSemaphore : RHICommand<RHICommandAddWaitSemaphore>
{
    RHISemaphore* semaphore;
    RHIPipelineStage pipeline_stage;

    RHICommandAddWaitSemaphore(RHISemaphore* in_semaphore, RHIPipelineStage in_pipeline_stage)
        : semaphore(in_semaphore), pipeline_stage(in_pipeline_stage)
    {
    }

    void execute_internal(RHICommandListContext& ctx)
    {
        ctx.RHIAddWaitSemaphore(semaphore, pipeline_stage);
    }
};

struct RHICommandAddSignalSemaphore : RHICommand<RHICommandAddSignalSemaphore>
{
    RHISemaphore* semaphore;
    RHIPipelineStage pipeline_stage;

    RHICommandAddSignalSemaphore(RHISemaphore* in_semaphore, RHIPipelineStage in_pipeline_stage)
        : semaphore(in_semaphore), pipeline_stage(in_pipeline_stage)
    {
    }

    void execute_internal(RHICommandListContext& ctx)
    {
        ctx.RHIAddSignalSemaphore(semaphore, pipeline_stage);
    }
};

struct RHICommandSetShaderParameters : RHICommand<RHICommandSetShaderParameters>
{
    std::vector<uint8_t> Payload;

    RHICommandSetShaderParameters(const std::vector<uint8_t> InPayload)
		: Payload(InPayload)
	{
	}

	void execute_internal(RHICommandListContext& cmd_list_ctx)
	{
		cmd_list_ctx.RHISetShaderParameters(Payload.data(), Payload.size());
	}
};

struct RHICommandDebugMarkTimelineBegin : RHICommand<RHICommandDebugMarkTimelineBegin>
{
    const ANSICHAR* Label;

    RHICommandDebugMarkTimelineBegin(const ANSICHAR* InLabel)
        : Label(InLabel)
    {
    }

    void execute_internal(RHICommandListContext& Ctx)
    {
        Ctx.RHIDebugMarkTimelineBegin(Label);
    }
};

struct RHICommandDebugMarkTimelineEnd : RHICommand<RHICommandDebugMarkTimelineEnd>
{
    RHICommandDebugMarkTimelineEnd()
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.RHIDebugMarkTimelineEnd();
    }
};