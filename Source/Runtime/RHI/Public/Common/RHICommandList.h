#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "InlineArray.h"
#include "RHIBuffer.h"
#include "RHITexture.h"
#include "Math/Vector2.h"

class RHISwapchain;
struct RHISemaphore;
struct RHIBuffer;
struct RHIPipeline;
struct RHITexture;
struct RHITextureView;
enum class RHITextureLayout : uint8_t;
enum class RHIResourceAccess : uint16_t;
enum class RHIPipelineStage : uint16_t;

class RHICommandListContext
{
public:
    virtual ~RHICommandListContext() = default;

    virtual RHICommandListContext& get_lowest_level_context() { return *this; }

    virtual void rhi_bind_pipeline(RHIPipeline *pipeline) = 0;
    virtual void rhi_set_scissor_rect(Vector2i position, Vector2i extent) = 0;
    virtual void rhi_pipeline_barrier_texture(RHITexture *texture, const RHITextureAccessState& from_texture_access_state, const RHITextureAccessState& to_texture_access_state) = 0;
    virtual void rhi_pipeline_barrier_buffer(RHIBuffer *buffer, const RHIBufferAccessState& from_buffer_access_state, const RHIBufferAccessState& to_buffer_access_state) = 0;
    virtual void rhi_copy_buffer(RHIBuffer* src, RHIBuffer* dst, uint32_t src_offset, uint32_t dst_offset, uint32_t size) = 0;
    virtual void rhi_copy_image(RHITexture* src, RHITexture* dst) = 0;
    virtual void rhi_begin_rendering(std::span<RHITextureView*> color_attachment_view, RHITextureView* depth_stencil_attachment, bool bClearColor) = 0;
    virtual void rhi_end_rendering() = 0;
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
    void enqueue(Args&&... args)
    {
        static_assert(std::is_base_of_v<RHICommandBase, Command>);
        size_t Offset = buffer.size();
        buffer.resize(Offset + sizeof(Command));
        new (buffer.data() + Offset) Command(std::forward<Args>(args)...);
    }

    std::vector<uint8_t> flush()
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
    static void execute(RHICommandListContext& cmd_list_ctx, std::vector<uint8_t>&& buffer)
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

struct RHICommandPipelineBarrierTexture : RHICommand<RHICommandPipelineBarrierTexture>
{
    RHITexture* texture;
    RHITextureAccessState from_texture_access_state;
    RHITextureAccessState to_texture_access_state;

    RHICommandPipelineBarrierTexture(RHITexture* in_texture,
        RHITextureAccessState in_from_texture_access_state,
        RHITextureAccessState in_to_texture_access_state)
        : texture(in_texture),
        from_texture_access_state(in_from_texture_access_state),
        to_texture_access_state(in_to_texture_access_state)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.rhi_pipeline_barrier_texture(texture, from_texture_access_state, to_texture_access_state);
    }
};

struct RHICommandPipelineBarrierBuffer : RHICommand<RHICommandPipelineBarrierBuffer>
{
    RHIBuffer* buffer;
    RHIBufferAccessState from_buffer_state;
    RHIBufferAccessState to_buffer_state;

    RHICommandPipelineBarrierBuffer(RHIBuffer* in_buffer, RHIBufferAccessState in_from_buffer_state, RHIBufferAccessState in_to_buffer_state)
        : buffer(in_buffer),
        from_buffer_state(in_from_buffer_state),
        to_buffer_state(in_to_buffer_state)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.rhi_pipeline_barrier_buffer(buffer, from_buffer_state, to_buffer_state);
    }
};

struct RHICommandBeginRendering : RHICommand<RHICommandBeginRendering>
{
    TInlineArray<RHITextureView*, 8> color_attachment_view;
    RHITextureView* depth_attachment_view;
    bool clear_color;

    RHICommandBeginRendering(std::span<RHITextureView*> in_color_attachment_view, RHITextureView* in_depth_attachment_view, bool in_clear_color)
        : color_attachment_view(in_color_attachment_view), depth_attachment_view(in_depth_attachment_view), clear_color(in_clear_color)
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.rhi_begin_rendering(color_attachment_view, depth_attachment_view, clear_color);
    }
};

struct RHICommandEndRendering : RHICommand<RHICommandEndRendering>
{
    RHICommandEndRendering()
    {
    }

    void execute_internal(RHICommandListContext& cmd_list_ctx)
    {
        cmd_list_ctx.rhi_end_rendering();
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
        cmd_list_ctx.rhi_bind_pipeline(pipeline);
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
        cmd_list_ctx.rhi_copy_buffer(src_buffer, dst_buffer, src_offset, dst_offset, size);
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
        cmd_list_ctx.rhi_copy_image(src, dst);
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
        cmd_list_ctx.rhi_set_scissor_rect(position, extent);
    }
};