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

        VkBuffer            buffer              = nullptr;
        VmaAllocation       allocation          = nullptr;
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

        void DestroyBuffer(GPUBuffer* buffer) override;
        void DestroyTexture(GPUTexture* texture) override;
        void DestroySampler(GPUSampler* sampler) override;
        void DestroyShader(GPUShader* shader) override;
        void DestroyPipeline(GPUPipeline* pipeline) override;
        void DestroyBindGroup(GPUBindGroup* bind_group) override;
    };
}