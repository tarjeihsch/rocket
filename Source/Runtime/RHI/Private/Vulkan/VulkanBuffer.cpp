#include "VulkanBuffer.h"

#include "Assertion.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"

#include <cstring>

#include "VulkanDescriptorManager.h"

VulkanBuffer::VulkanBuffer(VulkanDevice &in_device, const RHIBufferDesc& in_buffer_desc)
    : RHIBuffer(in_buffer_desc, RHIBufferAccessState::Default()),
    device(in_device),
    buffer_handle(nullptr),
    allocation_handle(nullptr),
    allocation_offset(0)
{
    Alloc();
}

VulkanBuffer::VulkanBuffer(VulkanDevice &in_device, const RHIBufferDesc& in_buffer_desc, VkBuffer in_buffer_handle)
    : RHIBuffer(in_buffer_desc, RHIBufferAccessState::Default()),
    device(in_device),
    buffer_handle(in_buffer_handle),
    allocation_handle(nullptr),
    allocation_offset(0)
{}

VulkanBuffer::~VulkanBuffer()
{
    Free();
}

void *VulkanBuffer::get_handle() const
{
    return buffer_handle;
}

uint32_t VulkanBuffer::GetBindingIndex() const
{
    return descriptor.Index;
}

void VulkanBuffer::map(const void *Data, uint64_t Size, uint64_t DstOffset)
{
    ensure(Size <= desc.size, "Trying to map beyond buffer size.");

    if (desc.Lifetime == RHIResourceLifetime::Transient)
    {
        void* BasePtr = device.get_transient_resource_allocator()->GetMappedBasePtr();
        void* DestPtr = static_cast<uint8_t*>(BasePtr) + allocation_offset + DstOffset;

        memcpy(DestPtr, Data, Size);
    }
    else if (desc.Lifetime == RHIResourceLifetime::Persistent)
    {
        void* DataPtr = nullptr;

        VkResult Result = vmaMapMemory(device.get_vma_allocator(), allocation_handle, &DataPtr);
        ensure(Result == VK_SUCCESS);

        memcpy(DataPtr, Data, Size);

        vmaUnmapMemory(device.get_vma_allocator(), allocation_handle);
    }
}

void VulkanBuffer::Alloc()
{
    VkBufferCreateInfo CreateInfo = VulkanRHI::GetVkBufferCreateInfo(desc.size, VulkanRHI::GetVkBufferUsage(desc.Usage));

    if (desc.Lifetime == RHIResourceLifetime::Transient)
    {
        VkResult Result = VulkanRHI::vkCreateBuffer(device.get_vk_device(), &CreateInfo, nullptr, &buffer_handle);
        ensure(Result == VK_SUCCESS);

        device.get_transient_resource_allocator()->alloc_buffer(buffer_handle, allocation_offset);
    }
    else if (desc.Lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent buffer
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = VulkanRHI::ToVmaMemoryUsage(desc.Memory);

        VkResult Result = vmaCreateBuffer(device.get_vma_allocator(), &CreateInfo, &AllocationCreateInfo, &buffer_handle, &allocation_handle, nullptr);
        ensure(Result == VK_SUCCESS);
    }

    if ((desc.Usage & RHIBufferUsage::Uniform) == RHIBufferUsage::Uniform || (desc.Usage & RHIBufferUsage::Storage) == RHIBufferUsage::Storage)
    {
        descriptor = device.get_descriptor_manager()->add_storage_buffer(buffer_handle, desc.size);
    }
}

void VulkanBuffer::Free()
{
    if (desc.Lifetime == RHIResourceLifetime::Transient)
    {
        // Only destroy handle, memory will be freed externally
        device.get_deferred_deletion_queue()->enqueue_resource(FDeferredDeletionQueue::EType::Buffer, buffer_handle, descriptor);
    }
    else if (desc.Lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, free memory
        device.get_deferred_deletion_queue()->enqueue_resource_with_allocation(FDeferredDeletionQueue::EType::Buffer, buffer_handle, descriptor, allocation_handle);
    }
}
