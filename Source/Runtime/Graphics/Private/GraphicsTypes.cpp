#include "GraphicsTypes.hpp"

#include <format>

#include "GeometryTypes.hpp"
#include "GraphicsUtils.hpp"

static void UploadGeometry(RHI::GPUBuffer* vertex_buffer, RHI::GPUBuffer* index_buffer, Vertex* vertex_data, uint64_t vertex_data_size, Face* face_data, uint64_t face_data_size)
{
    RHI::GPUCommandList* command_list = RHI::GetRHI()->CreateCommandList({RHI::GPUQueueFlag::Graphics});

    command_list->Begin();

    uint64_t staging_size = vertex_data_size + face_data_size;
    RHI::GPUBuffer* staging_buffer = RHI::GetRHI()->CreateBuffer(RHI::GPUBufferDesc::UploadBuffer(staging_size));
    {
        void* mapped = staging_buffer->Lock();
        memcpy(static_cast<u8*>(mapped), vertex_data, vertex_data_size);
        memcpy(static_cast<u8*>(mapped) + vertex_data_size, face_data, face_data_size);
        staging_buffer->Unlock();
    }

    BarrierBatch<2, 0>()
        .Add(vertex_buffer, RHI::GPUAccessType::Undefined, RHI::GPUAccessType::TransferDst)
        .Add(index_buffer, RHI::GPUAccessType::Undefined, RHI::GPUAccessType::TransferDst)
        .Submit(command_list);

    command_list->CopyBufferToBuffer(staging_buffer, vertex_buffer, vertex_data_size, 0, 0);
    command_list->CopyBufferToBuffer(staging_buffer, index_buffer, face_data_size, vertex_data_size, 0);

    BarrierBatch<2, 0>()
        .Add(vertex_buffer, RHI::GPUAccessType::TransferDst, RHI::GPUAccessType::VertexShaderRead)
        .Add(index_buffer, RHI::GPUAccessType::TransferDst, RHI::GPUAccessType::VertexShaderRead)
        .Submit(command_list);

    command_list->End();
    RHI::GetRHI()->SubmitImmediate(command_list);


    RHI::GetRHI()->WaitIdle();
    delete staging_buffer;
    delete command_list;
}

static void UploadTexture(RHI::GPUTexture* texture, u8* pixel_data, uint32_t width, uint32_t height, uint32_t channel_count)
{
    RHI::GPUCommandList* command_list = RHI::GetRHI()->CreateCommandList({RHI::GPUQueueFlag::Graphics});

    command_list->Begin();

    uint64_t pixel_data_size = static_cast<uint64_t>(width) * height * 4;

    RHI::GPUBuffer* staging_buffer = RHI::GetRHI()->CreateBuffer(RHI::GPUBufferDesc::UploadBuffer(pixel_data_size));
    {
        void* mapped = staging_buffer->Lock();
        memcpy(mapped, pixel_data, pixel_data_size);
        staging_buffer->Unlock();
    }

    BarrierBatch<0, 1>()
        .Add(texture, RHI::GPUTextureLayout::Undefined, RHI::GPUTextureLayout::TransferDst)
        .Submit(command_list);

    command_list->CopyBufferToTexture(staging_buffer, texture, width, height, 0, 0);

    BarrierBatch<0, 1>()
        .Add(texture, RHI::GPUTextureLayout::TransferDst, RHI::GPUTextureLayout::ShaderReadOnly)
        .Submit(command_list);

    command_list->End();
    RHI::GetRHI()->SubmitImmediate(command_list);

    RHI::GetRHI()->WaitIdle();

    delete staging_buffer;
    delete command_list;
}

void LoadTexture2D(Texture2D* texture_2d)
{
    Bitmap* bitmap = texture_2d->bitmap.get();

    RHI::GPUTextureDesc texture_desc = RHI::GPUTextureDesc::Texture2D(RHI::GPUFormat::RGBA8_UNORM, bitmap->width, bitmap->height, RHI::GPUTextureFlag::TransferDst | RHI::GPUTextureFlag::SRV);

    texture_2d->image.reset(RHI::GetRHI()->CreateTexture(texture_desc));
    texture_2d->image->SetDebugName("Texture2D");

    texture_2d->sampler.reset(RHI::GetRHI()->CreateSampler(RHI::GPUSamplerDesc::LinearRepeat()));

    UploadTexture(texture_2d->image.get(), bitmap->data, bitmap->width, bitmap->height, bitmap->channel_count);

    uint64_t texture_size = static_cast<uint64_t>(bitmap->width) * bitmap->height * bitmap->channel_count;
    std::printf("[Graphics] Loaded Texture:\n\tDimensions: %dx%d\n\tSize: %llu KB\n",
        bitmap->width,
        bitmap->height,
        texture_size / 1024
    );
}

void UnloadTexture2D(Texture2D *texture)
{
    texture->image = nullptr;
    texture->sampler = nullptr;
}

void CreateRenderTarget(RenderTarget *render_target)
{
    RHI::GPUTextureDesc color_target_texture_desc = RHI::GPUTextureDesc::RenderTarget(RHI::GPUFormat::RGBA16_FLOAT, render_target->width, render_target->height);
    RHI::GPUTextureDesc depth_target_texture_desc = RHI::GPUTextureDesc::DepthTarget(RHI::GPUFormat::D32_FLOAT, render_target->width, render_target->height);

    render_target->color_target_texture.reset(RHI::GetRHI()->CreateTexture(color_target_texture_desc));
    render_target->depth_target_texture.reset(RHI::GetRHI()->CreateTexture(depth_target_texture_desc));

#if 1
    render_target->color_target_texture->SetDebugName(std::format("(RenderTarget) Color: {}x{}", render_target->width, render_target->height).c_str());
    render_target->depth_target_texture->SetDebugName(std::format("(RenderTarget) Depth: {}x{}", render_target->width, render_target->height).c_str());

    std::printf("[Graphics] (Render Target [%p]):\n"
        "\tSize of color image: %d (MB)\n"
        "\tSize of depth image: %d (MB)\n",
        render_target,
        (render_target->width * render_target->height * RHI::GetFormatBytesPerPixel(RHI::GPUFormat::RGBA16_FLOAT)) / 1024 / 1024,
        (render_target->width * render_target->height * RHI::GetFormatBytesPerPixel(RHI::GPUFormat::D32_FLOAT)) / 1024 / 1024);
#endif
}

void FreeRenderTarget(RenderTarget *render_target)
{
    render_target->color_target_texture = nullptr;
    render_target->depth_target_texture = nullptr;

    std::printf("[Graphics] (Render Target [%p]) Freeing allocated RHI resources\n", render_target);
}

void LoadModel(Model *model)
{
    size_t vertex_buffer_size = model->surface->vertices.size() * sizeof(Vertex);
    size_t index_buffer_size = model->surface->faces.size() * sizeof(Face);
    
    model->vertex_buffer.reset(RHI::GetRHI()->CreateBuffer(RHI::GPUBufferDesc::DefaultBuffer(vertex_buffer_size, RHI::GPUBufferFlag::Vertex | RHI::GPUBufferFlag::Storage)));
    model->index_buffer.reset(RHI::GetRHI()->CreateBuffer(RHI::GPUBufferDesc::DefaultBuffer(index_buffer_size, RHI::GPUBufferFlag::Index | RHI::GPUBufferFlag::Storage)));

    model->vertex_buffer->SetDebugName("(Device) Vertex Buffer");
    model->index_buffer->SetDebugName("(Device) Index Buffer");
    
    UploadGeometry(model->vertex_buffer.get(), model->index_buffer.get(), model->surface->vertices.data(), vertex_buffer_size, model->surface->faces.data(), index_buffer_size);
    
    std::printf("[Graphics] Loaded Model:\n\tVertex Count (Size): %zu (%llu KB)\n\tFace Count (Size): %zu (%llu KB)\n",
        model->surface->vertices.size(),
        vertex_buffer_size / 1024,
        model->surface->faces.size(),
        index_buffer_size / 1024
    );
}

void UnloadModel(Model *model)
{
    model->vertex_buffer = nullptr;
    model->index_buffer = nullptr;
}

void CreateGraphicsPipeline(GraphicsPipeline *graphics_pipeline)
{
}

void CreateComputePipeline(ComputePipeline *compute_pipeline)
{
}

void FreeGraphicsPipeline(GraphicsPipeline *graphics_pipeline)
{
    if (graphics_pipeline == nullptr)
    {
        return;
    }

    // Pipelines reference their shaders, so release the pipeline before its shaders.
    graphics_pipeline->pipeline_handle.reset();
    graphics_pipeline->fragment_shader.reset();
    graphics_pipeline->vertex_shader.reset();
    delete graphics_pipeline;
}

void FreeComputePipeline(ComputePipeline *compute_pipeline)
{
    if (compute_pipeline == nullptr)
    {
        return;
    }

    compute_pipeline->pipeline_handle.reset();
    compute_pipeline->fragment_shader.reset();
    delete compute_pipeline;
}

void CreateLinearBuffer(LinearBuffer *linear_buffer)
{
    assert(linear_buffer->capacity > 0);

    linear_buffer->buffer.reset(RHI::GetRHI()->CreateBuffer(RHI::GPUBufferDesc::StorageBuffer(linear_buffer->capacity)));
}

void FreeLinearBuffer(LinearBuffer *linear_buffer)
{
    linear_buffer->buffer.release();
}

u64 WriteLinearBuffer(LinearBuffer *linear_buffer, const void *data, u64 size)
{
    assert(linear_buffer->size + size < linear_buffer->capacity);

    u64 offset = linear_buffer->size;

    void* ptr = linear_buffer->buffer->Lock();
    memcpy(static_cast<u8*>(ptr) + offset, data, size);
    linear_buffer->buffer->Unlock();
    
    linear_buffer->size += size;

    return offset;
}

void ResetLinearBuffer(LinearBuffer *linear_buffer)
{
    linear_buffer->size = 0;
}

void CreatePageBuffer(PageBuffer* page_buffer, u64 page_capacity, u64 alignment)
{
    u64 aligned_capacity = (page_capacity + alignment - 1) & ~(alignment - 1);

    u64 total_size = 0;
    for (u32 index = 0; index < 3; ++index)
    {
        page_buffer->page_capacity[index] = aligned_capacity;
        page_buffer->page_base[index] = total_size;
        total_size += aligned_capacity;
    }

    page_buffer->buffer.reset(RHI::GetRHI()->CreateBuffer(RHI::GPUBufferDesc::StorageBuffer(total_size)));
    page_buffer->buffer->SetDebugName("Page Buffer");

    memset(page_buffer->page_offset.data(), 0, sizeof(page_buffer->page_offset));
}

void FreePageBuffer(PageBuffer *page_buffer)
{
    page_buffer->buffer = nullptr;
}

void ResetPageBuffer(PageBuffer *page_buffer, u32 page_index)
{
    page_buffer->page_offset[page_index] = 0;
}

u64 WritePageBuffer(PageBuffer* page_buffer, u32 page_index, const void* data, u64 size, u64 alignment)
{
    assert(page_index < 3);

    u64 current = page_buffer->page_offset[page_index];
    u64 aligned = (current + alignment - 1) & ~(alignment - 1);

    assert(aligned + size <= page_buffer->page_capacity[page_index]);

    u64 byte_offset = page_buffer->page_base[page_index] + aligned;
    page_buffer->page_offset[page_index] = aligned + size;

    void* mapped = page_buffer->buffer->Lock();
    memcpy(static_cast<u8*>(mapped) + byte_offset, data, size);
    page_buffer->buffer->Unlock();

    return page_buffer->buffer->GetVirtualAddress() + byte_offset;
}
