#include "VulkanBuffer.h"

#include "Assertion.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanDescriptorManager.h"

#include <cstring>

VulkanBuffer::VulkanBuffer(VulkanDevice &in_device, const RHIBufferDesc& in_buffer_desc)
    : RHIBuffer(in_buffer_desc, RHIBufferAccessState::Default()),
    Device(in_device),
    BufferHandle(nullptr),
    AllocationHandle(nullptr),
    AllocationOffset(0)
{
    Alloc();
}

VulkanBuffer::VulkanBuffer(VulkanDevice &in_device, const RHIBufferDesc& in_buffer_desc, VkBuffer in_buffer_handle)
    : RHIBuffer(in_buffer_desc, RHIBufferAccessState::Default()),
    Device(in_device),
    BufferHandle(in_buffer_handle),
    AllocationHandle(nullptr),
    AllocationOffset(0)
{}

VulkanBuffer::~VulkanBuffer()
{
    Free();
}

void *VulkanBuffer::GetNativeBuffer() const
{
    return BufferHandle;
}

void VulkanBuffer::Map(const void *Data, uint64_t Size, uint64_t DstOffset)
{
    ensure(Size <= GetBufferDesc().size, "Trying to map beyond buffer size.");

    if (GetBufferDesc().Lifetime == RHIResourceLifetime::Transient)
    {
        void* BasePtr = Device.GetTransientCPUAllocator()->GetMappedBasePtr();
        void* DestPtr = static_cast<uint8_t*>(BasePtr) + AllocationOffset + DstOffset;

        memcpy(DestPtr, Data, Size);
    }
    else if (GetBufferDesc().Lifetime == RHIResourceLifetime::Persistent)
    {
        void* DataPtr = nullptr;

        VkResult Result = vmaMapMemory(Device.GetPersistentMemoryAllocator(), AllocationHandle, &DataPtr);
        ensure(Result == VK_SUCCESS);

        memcpy(DataPtr, Data, Size);

        vmaUnmapMemory(Device.GetPersistentMemoryAllocator(), AllocationHandle);
    }
}

void VulkanBuffer::Alloc()
{
    VkBufferCreateInfo CreateInfo = VulkanRHI::GetVkBufferCreateInfo(GetBufferDesc().size, VulkanRHI::GetVkBufferUsage(GetBufferDesc().Usage));

    if (GetBufferDesc().Lifetime == RHIResourceLifetime::Transient)
    {
        VkResult Result = VulkanRHI::vkCreateBuffer(Device.GetVkDevice(), &CreateInfo, nullptr, &BufferHandle);
        ensure(Result == VK_SUCCESS);

        Device.GetTransientCPUAllocator()->AllocBuffer(BufferHandle, AllocationOffset);
    }
    else if (GetBufferDesc().Lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent buffer
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = VulkanRHI::ToVmaMemoryUsage(GetBufferDesc().Memory);

        VkResult Result = vmaCreateBuffer(Device.GetPersistentMemoryAllocator(), &CreateInfo, &AllocationCreateInfo, &BufferHandle, &AllocationHandle, nullptr);
        ensure(Result == VK_SUCCESS);
    }

    VkBufferDeviceAddressInfo AddressInfo = {};
    AddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    AddressInfo.buffer = BufferHandle;
    Address = VulkanRHI::vkGetBufferDeviceAddress(Device.GetVkDevice(), &AddressInfo);

    if ((GetBufferDesc().Usage & RHIBufferUsage::Uniform) == RHIBufferUsage::Uniform || (GetBufferDesc().Usage & RHIBufferUsage::Storage) == RHIBufferUsage::Storage)
    {
        DescriptorHandle = Device.GetBindlessDescriptorManager()->GetHeap().RegisterBuffer(BufferHandle, GetBufferDesc().size);
    }
}

void VulkanBuffer::Free()
{
    if (GetBufferDesc().Lifetime == RHIResourceLifetime::Transient)
    {
        // Only destroy handle, memory will be freed externally
        Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::Buffer, BufferHandle, DescriptorHandle);
    }
    else if (GetBufferDesc().Lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, free memory
        Device.GetDeferredDeletionQueue()->enqueue_resource_with_allocation(FDeferredDeletionQueue::EType::Buffer, BufferHandle, DescriptorHandle, AllocationHandle);
    }
}
