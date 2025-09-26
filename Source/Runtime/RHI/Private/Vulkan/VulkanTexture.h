#pragma once

#include "VulkanDescriptorManager.h"
#include "Common/RHITexture.h"
#include "vma/vk_mem_alloc.h"

class FVulkanDevice;

// Both persistent and transient textures will only ever live within the duration of this wrapper.
struct FVulkanTexture : FRHITexture
{
    FVulkanTexture(FVulkanDevice& InDevice);
    FVulkanTexture(FVulkanDevice& InDevice, FRHITextureDesc& InDesc);
    FVulkanTexture(FVulkanDevice& InDevice, FRHITextureDesc& InDesc, VkImage InImage);
    ~FVulkanTexture();

    virtual const FRHITextureDesc& RHIGetTextureDesc() const override;
    virtual void* GetHandle() const override;

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
    FVulkanTextureView(FVulkanDevice& InDevice, FRHITextureViewDesc& InDesc, FRHITexture* Texture);
    FVulkanTextureView(FVulkanDevice& InDevice, FRHITextureViewDesc& InDesc, FRHITexture* Texture, VkImageView InImageView);
    ~FVulkanTextureView();

    virtual const FRHITextureViewDesc& RHIGetTextureViewDesc() const override;
    virtual void* GetHandle() const override;
    virtual uint32_t GetBindingIndex() const override;

    FRHITexture* Texture;
    VkImageView ImageView;

private:
    void Alloc();
    void Free();

private:
    FRHITextureViewDesc Desc;
    FVulkanDevice& Device;

    RDescriptorHandle DescriptorHandle;
};