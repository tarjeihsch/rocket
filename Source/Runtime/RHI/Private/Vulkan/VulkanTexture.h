#pragma once

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

    void Alias();

    virtual const FRHITextureDesc& RHIGetTextureDesc() const override;
    virtual void* GetHandle() const;

    VkImage Image;
    VmaAllocation Allocation;

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
    FVulkanTextureView(FVulkanDevice& InDevice, FRHITextureViewDesc& InDesc, VkImage InImage);
    FVulkanTextureView(FVulkanDevice& InDevice, FRHITextureViewDesc& InDesc, VkImage InImage, VkImageView InImageView);
    ~FVulkanTextureView();

    virtual const FRHITextureViewDesc& RHIGetTextureViewDesc() const override;
    virtual void* GetHandle() const override;

    VkImage Image;
    VkImageView View;
    VkImageAspectFlags AspectFlags;

private:
    void Alloc();
    void Free();

private:
    FRHITextureViewDesc Desc;
    FVulkanDevice& Device;
};