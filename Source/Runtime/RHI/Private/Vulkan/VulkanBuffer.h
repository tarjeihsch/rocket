#pragma once

#include "Common/Buffer.h"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

class FVulkanDevice;

struct FVulkanBuffer : FRHIBuffer
{
public:
    FVulkanBuffer(FVulkanDevice& InDevice, FRHIBufferDesc InDesc);
    FVulkanBuffer(FVulkanDevice& InDevice, FRHIBufferDesc InDesc, VkBuffer InBuffer);
    ~FVulkanBuffer();

    virtual const FRHIBufferDesc& GetBufferDesc() const override;
    virtual void* GetHandle() const override;

    virtual void Map(const void* Data, uint64_t Size) override;

private:
    void Alloc();
    void Free();

private:
    VkBuffer Buffer;
    VmaAllocation Allocation;

    VkDeviceSize Offset;

    FRHIBufferDesc Desc;

private:
    FVulkanDevice& Device;
};