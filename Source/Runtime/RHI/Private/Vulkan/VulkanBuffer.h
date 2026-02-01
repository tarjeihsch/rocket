#pragma once

#include "Common/RHIBuffer.h"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "VulkanDescriptorManager.h"

class VulkanDevice;

struct VulkanBuffer : RHIBuffer
{
private:
    VulkanDevice& Device;

public:
    VulkanBuffer(VulkanDevice& in_device, const RHIBufferDesc& in_buffer_desc);
    VulkanBuffer(VulkanDevice& in_device, const RHIBufferDesc& in_buffer_desc, VkBuffer in_buffer_handle);
    ~VulkanBuffer();

    // Begin RHIBuffer interface
    virtual void* GetNativeBuffer() const override;
    virtual void Map(const void *Data, uint64_t Size, uint64_t DstOffset = 0) override;
    // End RHIBuffer interface

private:
    void Alloc();
    void Free();

private:
    VkBuffer BufferHandle;
    VmaAllocation AllocationHandle;
    VkDeviceSize AllocationOffset;
    RHIDescriptorHandle DescriptorHandle;
    uint64_t Address;
};