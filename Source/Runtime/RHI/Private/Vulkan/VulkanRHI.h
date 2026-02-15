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

    struct VulkanGPUShaderResources : GPUShaderResources
    {
        ~VulkanGPUShaderResources();

        void SetDebugName(const char* debug_name) override;
        void SetBuffer(const GPUBuffer* buffer, uint32_t position, uint32_t index) override;
        void SetTexture(const GPUTexture* texture, uint32_t position, uint32_t index) override;
        void SetSampler(const GPUSampler* sampler, uint32_t position, uint32_t index) override;

        VkDescriptorSet             descriptor_set              = nullptr;
        VkDescriptorSetLayout       descriptor_set_layout       = nullptr;
        VkDescriptorPool            descriptor_pool             = nullptr;
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
        VkPipelineLayout layout = nullptr;
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

    class VulkanRHI : public IRHI
    {
    public:
        VulkanRHI();
        ~VulkanRHI();

        GPUAdapter GetAdapater() const override;

        void Submit(GPUCommandList* command_list) override;
        void WaitIdle() override;

        GPUBuffer* CreateBuffer(const GPUBufferDesc& buffer_desc) override;
        GPUTexture* CreateTexture(const GPUTextureDesc& texture_desc) override;
        GPUSampler* CreateSampler(const GPUSamplerDesc& sampler_desc) override;
        GPUShaderResources* CreateBindGroup(const GPUBindGroupDesc& bind_group_desc) override;
        GPUPipeline* CreateGraphicsPipeline(const GPUGraphicsPipelineDesc& pipeline_desc) override;
        GPUShader* CreateShader(const GPUShaderDesc& pipeline_desc) override;
        GPUSwapchain* CreateSwapchain(const GPUSwapchainDesc& desc) override;
        GPUCommandList* CreateCommandList() override;
    };
}