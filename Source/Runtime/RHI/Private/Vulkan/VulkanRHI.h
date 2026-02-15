#pragma once

#include "Common/RHI.h"

#include "VulkanLoader.h"

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
                VkImageView uav;
            };

            VkImageView views[4] = {};
        };
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

    struct VulkanGPUSwapchain : GPUSwapchain
    {
        ~VulkanGPUSwapchain();

        void                    SetDebugName(const char* debug_name)    override;
        GPUTexture*             GetBackBufferTexture()                  const override;
        void                    BeginFrame()                            override;
        void                    PresentFrame()                          override;

        VkSwapchainKHR          swapchain                               = nullptr;
        VkSurfaceKHR            surface                                 = nullptr;
        uint32_t                image_count                             = 0;
        uint32_t                current_image_index                     = 0;
        uint32_t                current_frame                           = 0;
        uint32_t                frames_in_flight                        = 0;
        VulkanGPUTexture*       textures[3]                             = {};

        VkSemaphore             image_available[3]                      = {};
        VkSemaphore             render_finished[3]                      = {};
        VkFence                 in_flight_fences[3]                     = {};
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
        GPUPipeline* CreateGraphicsPipeline(const GPUGraphicsPipelineDesc& pipeline_desc) override;
        GPUShader* CreateShader(const GPUShaderDesc& pipeline_desc) override;
        GPUSwapchain* CreateSwapchain(const GPUSwapchainDesc& desc) override;
        GPUCommandList* CreateCommandList() override;

        void SetTexture(uint32_t index, const GPUTexture* texture) override;
        void SetBuffer(uint32_t index, const GPUBuffer* buffer) override;
        void SetSampler(uint32_t index, const GPUSampler* sampler) override;

        void Submit(GPUCommandList* command_list) override;
        void WaitIdle() override;
    };
}