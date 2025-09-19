#pragma once

#include "Common/RHIBuffer.h"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "VulkanDescriptorManager.h"

class FVulkanDevice;

struct FVulkanBuffer : FRHIBuffer
{
public:
    FVulkanBuffer(FVulkanDevice& InDevice, FRHIBufferDesc InDesc);
    FVulkanBuffer(FVulkanDevice& InDevice, FRHIBufferDesc InDesc, VkBuffer InBuffer);
    ~FVulkanBuffer();

    virtual const FRHIBufferDesc& GetBufferDesc() const override;
    virtual void* GetHandle() const override;

    virtual uint32_t GetBindingIndex() const override;

    virtual void Map(const void* Data, uint64_t Size, uint64_t DstOffset = 0) override;

private:
    void Alloc();
    void Free();

private:
    VkBuffer Buffer;
    VmaAllocation Allocation;
    VkDeviceSize Offset;

    DescriptorHandle m_DescriptorHandle;

    FRHIBufferDesc Desc;

private:
    FVulkanDevice& Device;
};