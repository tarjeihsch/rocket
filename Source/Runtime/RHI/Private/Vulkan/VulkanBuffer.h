#pragma once

#include "Common/RHIBuffer.h"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "VulkanDescriptorManager.h"

class VulkanDevice;

struct VulkanBuffer : RHIBuffer
{
private:
    VulkanDevice& device;

public:
    VulkanBuffer(VulkanDevice& in_device, const RHIBufferDesc& in_buffer_desc);
    VulkanBuffer(VulkanDevice& in_device, const RHIBufferDesc& in_buffer_desc, VkBuffer in_buffer_handle);
    ~VulkanBuffer();

    virtual void* GetNativeBuffer() const override;
    virtual uint32_t GetBindingIndex() const override;
    virtual void Map(const void *Data, uint64_t Size, uint64_t DstOffset) override;

private:
    void Alloc();
    void Free();

private:
    VkBuffer buffer_handle;
    VmaAllocation allocation_handle;
    VkDeviceSize allocation_offset;
    BindlessHandle descriptor;
};