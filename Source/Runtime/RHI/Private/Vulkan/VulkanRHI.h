#pragma once

#include "Common/RHI.h"

#include "VulkanLoader.h"

#include <vector>

namespace RHI
{
    struct VulkanGPUBuffer : GPUBuffer
    {
        ~VulkanGPUBuffer();

        void SetDebugName(const char* debug_name) override;
        uint64_t GetVirtualAddress() const override;

        void* Lock() override;
        void Unlock() override;

        VkBuffer            buffer              = nullptr;
        VmaAllocation       allocation          = nullptr;
    };

    struct VulkanGPUTexture : GPUTexture
    {
        ~VulkanGPUTexture();

        void SetDebugName(const char* debug_name) override;

        VkImage             texture             = nullptr;
        VmaAllocation       allocation          = nullptr;
        
        union
        {
            struct
            {
                VkImageView rtv;
                VkImageView dsv;
                VkImageView srv;
                VkImageView srgb_srv;
            };

            VkImageView views[4] = {};
        };

        std::vector<VkImageView> uavs;
    };

    struct VulkanGPUSampler : GPUSampler
    {
        ~VulkanGPUSampler();

        void SetDebugName(const char* debugName) override;

        VkSampler sampler;
    };

    struct VulkanGPUShader : GPUShader
    {
        ~VulkanGPUShader();

        VkShaderModule module = nullptr;
        VkShaderStageFlagBits stage = VK_SHADER_STAGE_VERTEX_BIT;
    };

    struct VulkanGPUGraphicsPipeline : GPUGraphicsPipeline
    {
        ~VulkanGPUGraphicsPipeline();

        void SetDebugName(const char* debug_name) override;

        VkPipeline pipeline = nullptr;
    };

    struct VulkanGPUComputePipeline : GPUComputePipeline
    {
        ~VulkanGPUComputePipeline();

        void SetDebugName(const char* debug_name) override;

        VkPipeline pipeline = nullptr;
    };

    struct VulkanGPUSwapchain : GPUSwapchain
    {
        ~VulkanGPUSwapchain();

        void                    SetDebugName(const char* debug_name)    override;
        GPUTexture*             GetBackBufferTexture()                  const override;
        bool                    AcquireNextBackBuffer()                 override;
        bool                    PresentBackBuffer()                     override;
        void                    ResizeBackBuffers()                     override;
        Vector2i                GetBackBufferSize()                     override;

        VkSwapchainKHR          swapchain                               = nullptr;
        VkSurfaceKHR            surface                                 = nullptr;
        uint32_t                image_count                             = 0;
        uint32_t                current_image_index                     = 0;
        VulkanGPUTexture*       textures[3]                             = {};
        VkExtent2D              extent                                  = {};
        VkSemaphore*            image_available_semaphores              = nullptr;
        VkSemaphore*            render_finished_semaphores              = nullptr;
    };

    struct VulkanGPUCommandList : GPUCommandList
    {
        ~VulkanGPUCommandList();

        void Begin() override;
        void End() override;
        void BeginRenderPass(GPURenderPass* rendering_info) override;
        void EndRenderPass() override;
        void SetGraphicsPipeline(const GPUGraphicsPipeline* pipeline) override;
        void SetComputePipeline(const GPUComputePipeline* computePipeline) override;
        void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) override;
        void SetScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;
        void SetPushConstants(const void* data, uint32_t size) override;
        void SetIndexBuffer(const GPUBuffer* buffer, uint64_t offset) override;
        void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) override;
        void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset, uint32_t first_instance) override;
        void DrawIndirect(const GPUBuffer* buffer, uint64_t offset, uint32_t draw_count) override;
        void DrawIndexedIndirectCount(const GPUBuffer* buffer, uint64_t offset, const GPUBuffer* count_buffer, uint64_t count_offset, uint32_t max_draw_count) override;
        void Dispatch(uint32_t x, uint32_t y, uint32_t z) override;
        void FillBuffer(GPUBuffer* buffer, uint64_t size, uint64_t offset, uint32_t value) override;
        void CopyBufferToBuffer(GPUBuffer* src, GPUBuffer* dst, uint64_t size, uint64_t src_offset, uint64_t dst_offset) override;
        void CopyBufferToTexture(GPUBuffer* src, GPUTexture* dst, uint32_t width, uint32_t height, uint32_t mip, uint32_t layer) override;
        void CopyTextureToTexture(GPUTexture* src, GPUTexture* dst, uint32_t width, uint32_t height) override;
        void Barrier(const GPUBufferBarrier* buffer_barriers, uint32_t buffer_barrier_count, const GPUTextureBarrier* texture_barriers, uint32_t texture_barrier_count) override;
        uint32_t BeginTimelineQuery() override;
        uint32_t EndTimelineQuery() override;
        void BeginPipelineQuery() override;
        void EndPipelineQuery() override;

        VkCommandBuffer command_buffer_data[3] = {};
        VkCommandPool command_pool_data[3] = {};
    };

    class VulkanRHI final : public IRHI
    {
    public:
        VulkanRHI();
        ~VulkanRHI();

        GPUAdapter GetAdapater() const override;

        GPUBuffer* CreateBuffer(const GPUBufferDesc& buffer_desc) override;
        GPUTexture* CreateTexture(const GPUTextureDesc& texture_desc) override;
        GPUSampler* CreateSampler(const GPUSamplerDesc& sampler_desc) override;
        GPUGraphicsPipeline* CreateGraphicsPipeline(const GPUGraphicsPipelineDesc& pipeline_desc) override;
        GPUComputePipeline* CreateComputePipeline(const GPUComputePipelineDesc& pipeline_desc) override;
        GPUShader* CreateShader(const GPUShaderDesc& pipeline_desc) override;
        GPUSwapchain* CreateSwapchain(const GPUSwapchainDesc& desc) override;
        GPUCommandList* CreateCommandList(const GPUCommandListDesc& desc) override;

        void SetTexture(uint32_t index, const GPUTexture* texture) override;
        void SetRWTexture(uint32_t index, const GPUTexture* texture, uint32_t mip) override;
        void SetBuffer(uint32_t index, const GPUBuffer* buffer) override;
        void SetSampler(uint32_t index, const GPUSampler* sampler) override;

        void Submit(GPUCommandList* command_list, GPUSwapchain* swapchain) override;
        void SubmitImmediate(GPUCommandList* command_list) override;
        void WaitIdle() override;

        GPUTimingQueryResult GetTimingQueryResult() override;
        uint32_t GetTimestampResults(double* milliseconds, uint32_t capacity) override;
        GPUPipelineQueryResult GetPipelineQueryResult() override;
    };
}
