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

    struct VulkanGPUBindGroup : GPUBindGroup
    {
        ~VulkanGPUBindGroup();

        void SetDebugName(const char* debug_name) override;
        void SetBuffer(uint32_t bind_point, const GPUBuffer* buffer) override;
        void SetTexture(uint32_t bind_point, const GPUTexture* texture) override;
        void SetSampler(uint32_t bind_point, const GPUSampler* sampler) override;

        VkDescriptorSet             descriptor_set              = nullptr;
        VkDescriptorSetLayout       descriptor_set_layout       = nullptr;
        VkDescriptorPool            descriptor_pool             = nullptr;
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
        GPUBindGroup* CreateBindGroup(const GPUBindGroupDesc& bind_group_desc) override;
        GPUPipeline* CreateGraphicsPipeline(const GPUGraphicsPipelineDesc& pipeline_desc) override;
        GPUShader* CreateShader(const GPUShaderDesc& pipeline_desc) override;
        GPUSwapchain* CreateSwapchain(const GPUSwapchainDesc& desc) override;
        GPUCommandList* CreateCommandList() override;
    };
}