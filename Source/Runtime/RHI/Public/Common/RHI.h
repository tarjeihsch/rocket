#pragma once

#include "Common/RHITypes.h"

namespace RHI
{
    // RHI Resource Ownership
    // All RHI resources (buffers, textures, etc.) are created via IRHI factory methods
    // (e.g. CreateBuffer) and destroyed by deleting the returned pointer. Resources
    // clean up their underlying API objects in their destructors. There are no
    // explicit Destroy methods on the RHI interface.

    struct GPUBuffer
    {
        virtual ~GPUBuffer() = default;

        virtual void SetDebugName(const char* debug_name) = 0;
        virtual uint64_t GetVirtualAddress() const = 0;

        virtual void* Lock() = 0;
        virtual void Unlock() = 0;
    };

    struct GPUBufferView
    {
        virtual ~GPUBufferView() = default;
    };

    struct GPUTexture
    {
        virtual ~GPUTexture() = default;
        
        virtual void SetDebugName(const char* debug_name) = 0;
    };
    
    struct GPUSampler
    {
        virtual ~GPUSampler() = default;

        virtual void SetDebugName(const char* debug_name) = 0;
    };

    struct GPUPipeline
    {
        virtual ~GPUPipeline() = default;
        
        virtual void SetDebugName(const char* debug_name) = 0;
    };

    struct GPUGraphicsPipeline : GPUPipeline
    {
        virtual ~GPUGraphicsPipeline() = default;
    };

    struct GPUComputePipeline : GPUPipeline
    {
        virtual ~GPUComputePipeline() = default;
    };

    struct GPUShader
    {
        virtual ~GPUShader() = default;
    };

    struct GPUSwapchain
    {
        virtual ~GPUSwapchain() = default;

        virtual void SetDebugName(const char* debug_name) = 0;
        virtual GPUTexture* GetBackBufferTexture() const = 0;
        virtual bool AcquireNextBackBuffer() = 0;
        virtual void PresentBackBuffer() = 0;
        virtual void ResizeBackBuffers() = 0;
        virtual Vector2i GetBackBufferSize() = 0;
    };

    struct GPUCommandList
    {
        virtual ~GPUCommandList() = default;

        virtual void Begin() = 0;
        virtual void End() = 0;
        virtual void BeginRenderPass(GPURenderPass* rendering_info) = 0;
        virtual void EndRenderPass() = 0;
        virtual void SetPipeline(const GPUPipeline* pipeline) = 0;
        virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
        virtual void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) = 0;
        virtual void SetPushConstants(const void* data, uint32_t size) = 0;
        virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) = 0;
        virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) = 0;
        virtual void DrawIndirect(const GPUBuffer* buffer, uint64_t offset, uint32_t draw_count) = 0;
        virtual void Dispatch(uint32_t x, uint32_t y, uint32_t z) = 0;
        virtual void CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, uint64_t size, uint64_t src_offset, uint64_t dst_offset) = 0;
        virtual void CopyBufferToTexture(GPUBuffer* src, GPUTexture* dst, uint32_t mip, uint32_t layer) = 0;
        virtual void CopyTextureToTexture(GPUTexture* src, GPUTexture* dst, uint32_t width, uint32_t height) = 0;
        virtual void Barrier(const GPUBufferBarrier* buffer_barriers, uint32_t buffer_barrier_count, const GPUTextureBarrier* texture_barriers, uint32_t texture_barrier_count) = 0;
        virtual void BeginTimelineQuery() = 0;
        virtual void EndTimelineQuery() = 0;
        virtual void BeginPipelineQuery() = 0;
        virtual void EndPipelineQuery() = 0;

        void Barrier(const GPUBufferBarrier& barrier)
        {
            Barrier(&barrier, 1, nullptr, 0);
        }

        void Barrier(const GPUTextureBarrier& barrier)
        {
            Barrier(nullptr, 0, &barrier, 1);
        }
    };

	class IRHI
	{
    public:
        virtual ~IRHI() = default;

        virtual GPUAdapter GetAdapater() const = 0;

        virtual GPUBuffer* CreateBuffer(const GPUBufferDesc& buffer_desc) = 0;
        virtual GPUTexture* CreateTexture(const GPUTextureDesc& texture_desc) = 0;
        virtual GPUSampler* CreateSampler(const GPUSamplerDesc& sampler_desc) = 0;
        virtual GPUPipeline* CreateGraphicsPipeline(const GPUGraphicsPipelineDesc& pipeline_desc) = 0;
        virtual GPUShader* CreateShader(const GPUShaderDesc& pipeline_desc) = 0;
        virtual GPUSwapchain* CreateSwapchain(const GPUSwapchainDesc& desc) = 0;
        virtual GPUCommandList* CreateCommandList(const GPUCommandListDesc& desc) = 0;

        virtual void SetTexture(uint32_t index, const GPUTexture* texture) = 0;
        virtual void SetBuffer(uint32_t index, const GPUBuffer* buffer) = 0;
        virtual void SetSampler(uint32_t index, const GPUSampler* sampler) = 0;

        virtual void Submit(GPUCommandList* command_list) = 0;
        virtual void WaitIdle() = 0;

        virtual GPUTimingQueryResult GetTimingQueryResult() = 0;
        virtual GPUPipelineQueryResult GetPipelineQueryResult() = 0;
	};

    void Initialize(GPUDriver driver);
    void Shutdown();

    extern IRHI* g_rhi;

    inline IRHI* GetRHI()
    {
        return g_rhi;
    }

    template<typename T>
    T* CastRHI() 
    {
        return static_cast<T>(GetRHI());
    }

    static constexpr uint32_t MAX_COLOR_ATTACHMENT_COUNT = 8;
    static constexpr uint32_t CONCURRENT_FRAME_COUNT = 3;

    static uint64_t g_frame_counter = 0;

    static bool is_minimized = false;

    #define CURRENT_FRAME g_frame_counter % CONCURRENT_FRAME_COUNT
};