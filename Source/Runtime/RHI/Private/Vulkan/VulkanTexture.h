#pragma once

#include "Common/RHITexture.h"
#include "vma/vk_mem_alloc.h"

class FVulkanDevice;

struct FVulkanTexture : FRHITexture
{
    FVulkanTexture(FVulkanDevice& InDevice, FRHITextureDesc& InDesc);
    FVulkanTexture(FVulkanDevice& InDevice, FRHITextureDesc& InDesc, VkImage InImage);
    ~FVulkanTexture();

    virtual const FRHITextureDesc& RHIGetTextureDesc() const override;

    VkImage Image;
    VmaAllocation Allocation;

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

    VkImage Image;
    VkImageView View;
    VkImageAspectFlags AspectFlags;

private:
    void Alloc();
    void Free();

private:
    FVulkanDevice& Device;
    FRHITextureViewDesc Desc;
};