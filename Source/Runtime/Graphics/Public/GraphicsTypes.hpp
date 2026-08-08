#pragma once

#include <memory>
#include <array>
#include <unordered_map>
#include <Common/RHI.h>

struct Mesh;

struct Bitmap
{
    u8* data;

    u32 width;
    
    u32 height;
    
    u32 channel_count;
};

struct Texture2D
{
    std::shared_ptr<Bitmap> bitmap;

    std::unique_ptr<RHI::GPUTexture> image;

    std::unique_ptr<RHI::GPUSampler> sampler;
};

struct RenderTarget
{
    std::unique_ptr<RHI::GPUTexture> color_target_texture;
    
    std::unique_ptr<RHI::GPUTexture> depth_target_texture;

    u32 width;

    u32 height;
};

struct Material
{
    std::shared_ptr<Texture2D> texture_map_albedo;
    
    std::shared_ptr<Texture2D> texture_map_roughness;
    
    std::shared_ptr<Texture2D> texture_map_metallic;

    std::shared_ptr<Texture2D> texture_map_ao;
};

struct Model
{
    // CPU-side mesh/surface data used to build and describe the model geometry.
    std::shared_ptr<Mesh> surface;

    std::shared_ptr<Material> material;

    // GPU buffer containing vertex data (positions, normals, UVs, etc.).
    std::unique_ptr<RHI::GPUBuffer> vertex_buffer;

    // GPU buffer containing triangle indices referencing vertices.
    std::unique_ptr<RHI::GPUBuffer> index_buffer;
};

struct GraphicsPipeline
{
    std::unique_ptr<RHI::GPUShader> vertex_shader;

    std::unique_ptr<RHI::GPUShader> fragment_shader;
    
    std::unique_ptr<RHI::GPUGraphicsPipeline> pipeline_handle;

    u64 hash;
};

struct ComputePipeline
{
    std::unique_ptr<RHI::GPUShader> fragment_shader;
    
    std::unique_ptr<RHI::GPUComputePipeline> pipeline_handle;

    u64 hash;
};

struct Glyph
{
    u32 unicode;
    
    float advance;
    
    float plane_bounds_left;
    
    float plane_bounds_bottom;
    
    float plane_bounds_right;
    
    float plane_bounds_top;
    
    float atlas_bounds_left;
    
    float atlas_bounds_bottom;
    
    float atlas_bounds_right;
    
    float atlas_bounds_top;
};

struct Font
{
    std::unordered_map<u32, Glyph> glyphs;
    
    std::shared_ptr<Texture2D> atlas;
    
    float width;
    
    float height;
    
    float scale;
    
    float range;
};

struct LinearBuffer
{
    std::unique_ptr<RHI::GPUBuffer> buffer;

    u64 size;

    u64 capacity;
};

struct PageBuffer
{
    std::unique_ptr<RHI::GPUBuffer> buffer;

    std::array<u64, 3> page_base;

    std::array<u64, 3> page_capacity;

    std::array<u64, 3> page_offset;
};

void LoadTexture2D(Texture2D* texture);
void UnloadTexture2D(Texture2D* texture);

void CreateRenderTarget(RenderTarget* render_target);
void FreeRenderTarget(RenderTarget* render_target);

void LoadModel(Model* model);
void UnloadModel(Model* model);

void CreateGraphicsPipeline(GraphicsPipeline* graphics_pipeline);
void FreeGraphicsPipeline(GraphicsPipeline* graphics_pipeline);

void CreateComputePipeline(ComputePipeline* compute_pipeline);
void FreeComputePipeline(ComputePipeline* compute_pipeline);

void CreateLinearBuffer(LinearBuffer* linear_buffer);
void FreeLinearBuffer(LinearBuffer* linear_buffer);
void ResetLinearBuffer(LinearBuffer* linear_buffer);
u64 WriteLinearBuffer(LinearBuffer* linear_buffer, const void* data, u64 size);

void CreatePageBuffer(PageBuffer* buffer, u64 page_capacity, u64 alignment);
void FreePageBuffer(PageBuffer* buffer);
void ResetPageBuffer(PageBuffer* buffer, u32 index);
u64 WritePageBuffer(PageBuffer* buffer, u32 page_index, const void* data, u64 size, u64 alignment);