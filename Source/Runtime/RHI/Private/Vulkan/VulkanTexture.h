#pragma once

#include <vma/vk_mem_alloc.h>

#include "Common/RHITexture.h"
#include "VulkanDescriptorManager.h"

class VulkanSurface;
class FVulkanDevice;
struct RDescriptorHandle;

// Both persistent and transient textures will only ever live within the duration of this wrapper.
struct FVulkanTexture : RHITexture
{
    FVulkanTexture(FVulkanDevice& InDevice);
    FVulkanTexture(FVulkanDevice& InDevice, FRHITextureDesc& InDesc);
    FVulkanTexture(FVulkanDevice& InDevice, FRHITextureDesc& InDesc, VkImage InImage);
    ~FVulkanTexture();

    virtual const FRHITextureDesc& RHIGetTextureDesc() const override;
    virtual void* GetHandle() const override;
    virtual void gpu_barrier(RHICommandList& cmd_list, ERHITextureLayout new_layout, ERHIResourceAccess new_access, ERHIPipelineStage new_pipeline_stage) override;
    
    VkImage Image;
    VmaAllocation Allocation;
    uint64_t Offset;

    VkImageLayout ImageLayout;
    VkAccessFlags AccessFlags;
    VkPipelineStageFlags PipelineStageFlags;

private:
    void Alloc();
    void Free();

private:
    FRHITextureDesc Desc;
    FVulkanDevice& Device;
};

struct FVulkanTextureView : FRHITextureView
{
    FVulkanTextureView(FVulkanDevice& InDevice, FRHITextureViewDesc& InDesc, RHITexture* Texture);
    FVulkanTextureView(FVulkanDevice& InDevice, FRHITextureViewDesc& InDesc, RHITexture* Texture, VkImageView InImageView);
    ~FVulkanTextureView();

    virtual const FRHITextureViewDesc& RHIGetTextureViewDesc() const override;
    virtual void* GetHandle() const override;
    virtual uint32_t GetBindingIndex() const override;
    virtual RHITexture* GetTexture() const override;

    RHITexture* Texture;
    VkImageView ImageView;

private:
    void Alloc();
    void Free();

private:
    FRHITextureViewDesc Desc;
    FVulkanDevice& Device;
    RDescriptorHandle DescriptorHandle;
};
