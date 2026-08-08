#pragma once

#include <string>
#include <vector>

#include <Color.h>
#include <StaticArray.h>
#include <Math/Vector.h>
#include <Common/RHI.h>

struct GraphicsPipeline;
struct ComputePipeline;
struct Font;
struct Mesh;
struct PageBuffer;

//class MeshBuilder
//{
//public:
//    void AddTriangle();
//    void AddQuad();
//    
//private:
//    std::vector<Vertex> vertices;
//};

class TextBuilder
{
public:
    TextBuilder();

    TextBuilder& SetFont(const Font* font);
    TextBuilder& SetColor(Color32 color);
    TextBuilder& SetScale(float font_size);
    TextBuilder& SetText(const std::string& msg);

    Mesh Build() const;

private:
    std::string text;

    const Font* font;

    Color32 color;
    
    float scale;
};

class PrimitiveBuilder
{
};

class PipelineBuilder
{
public:
    PipelineBuilder();

    PipelineBuilder& SetVertexShader(const u8* data, u32 size);
    PipelineBuilder& SetFragmentShader(const u8* data, u32 size);
    PipelineBuilder& SetComputeShader(const u8* data, u32 size);

    PipelineBuilder& SetTriangleStrip();
    PipelineBuilder& SetTriangleList();
    PipelineBuilder& SetWireframeLine();
    PipelineBuilder& SetWireframeList();

    PipelineBuilder& SetCullMode(RHI::GPUCullMode mode);
    PipelineBuilder& SetFrontFace(RHI::GPUFrontFace face);

    PipelineBuilder& SetDepthMode(
        bool enable_test,
        bool enable_write,
        RHI::GPUCompareOp compare_op,
        RHI::GPUFormat format
    );
    
    PipelineBuilder& SetBlendMode(
        bool enable,
        RHI::GPUBlendFactor srcColor,
        RHI::GPUBlendFactor dstColor,
        RHI::GPUBlendFactor srcAlpha,
        RHI::GPUBlendFactor dstAlpha
    );

    PipelineBuilder& AddColorFormat(RHI::GPUFormat format);

    GraphicsPipeline* BuildGraphicsPipeline();
    ComputePipeline* BuildComputePipeline();

private:
    RHI::GPUGraphicsPipelineDesc graphics_pipeline_desc;
    RHI::GPUComputePipelineDesc compute_pipeline_desc;
    
    std::vector<u8> vertex_shader_data;
    std::vector<u8> fragment_shader_data;
    std::vector<u8> compute_shader_data;
};

class BufferBuilder
{
public:
    template <typename T>
    BufferBuilder& Add(const T* value, u32 count = 1)
    {
        data.Insert(value, sizeof(T) * count);
        return *this;
    }

    template <typename T>
    BufferBuilder& Add(const T& value)
    {
        data.Insert(&value, sizeof(T));
        return *this;
    }

    BufferBuilder& SetTarget(PageBuffer* page_buffer);

    u64 Build();

private:
    StaticArray<u8, 4096> data;

    PageBuffer* target;
};

template<size_t BufferCount = 8, size_t TextureCount = 8>
struct BarrierBatch
{
	constexpr BarrierBatch& Add(RHI::GPUBuffer* buffer, RHI::GPUAccessType from, RHI::GPUAccessType to)
	{
		assert(buffer_barrier_count < BufferCount);
		buffer_barrier_data[buffer_barrier_count++] = { buffer, from, to };
		return *this;
	}
	
	constexpr BarrierBatch& Add(RHI::GPUTexture* texture, RHI::GPUTextureLayout from, RHI::GPUTextureLayout to)
	{
		assert(texture_barrier_count < TextureCount);
		texture_barrier_data[texture_barrier_count++] = { texture, from, to };
		return *this;
	}

	void Submit(RHI::GPUCommandList* command_list) const
	{
		command_list->Barrier(buffer_barrier_data, buffer_barrier_count, texture_barrier_data, texture_barrier_count);
	}

private:
	RHI::GPUBufferBarrier buffer_barrier_data[BufferCount > 0 ? BufferCount : 1] = {};
	RHI::GPUTextureBarrier texture_barrier_data[TextureCount > 0 ? TextureCount : 1] = {};

	uint16_t buffer_barrier_count = 0;
	uint16_t texture_barrier_count = 0;
};
