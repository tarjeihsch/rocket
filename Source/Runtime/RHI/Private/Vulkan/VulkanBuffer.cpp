#include "VulkanBuffer.h"

#include "Assert.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"

#include <cstring>

#include "VulkanDescriptorManager.h"

FVulkanBuffer::FVulkanBuffer(FVulkanDevice &InDevice, FRHIBufferDesc InDesc)
    : Device(InDevice), Desc(InDesc)
{
    Alloc();
}

FVulkanBuffer::FVulkanBuffer(FVulkanDevice &InDevice, FRHIBufferDesc InDesc, VkBuffer InBuffer) : Device(InDevice), Desc(InDesc), Buffer(InBuffer)
{
}

FVulkanBuffer::~FVulkanBuffer()
{
    Free();
}

const FRHIBufferDesc &FVulkanBuffer::GetBufferDesc() const
{
    return Desc;
}

void *FVulkanBuffer::GetHandle() const
{
    return Buffer;
}

uint32_t FVulkanBuffer::GetBindingIndex() const
{
    return m_DescriptorHandle.Index;
}

void FVulkanBuffer::Map(const void *Data, uint64_t Size, uint64_t DstOffset)
{
    RK_ASSERT(Size <= Desc.Size, "Trying to map beyond buffer size.");

    if (Desc.Lifetime == ERHIResourceLifetime::Transient)
    {
        void* BasePtr = Device.GetTransientResourceAllocator()->GetMappedBasePtr();
        void* DestPtr = static_cast<uint8_t*>(BasePtr) + Offset + DstOffset;

        memcpy(DestPtr, Data, Size);
    }
    else if (Desc.Lifetime == ERHIResourceLifetime::Persistent)
    {
        void* DataPtr = nullptr;

        VkResult Result = vmaMapMemory(Device.GetVmaAllocator(), Allocation, &DataPtr);
        RK_ASSERT(Result == VK_SUCCESS);

        memcpy(DataPtr, Data, Size);

        vmaUnmapMemory(Device.GetVmaAllocator(), Allocation);
    }
}

void FVulkanBuffer::Alloc()
{
    VkBufferCreateInfo CreateInfo = VulkanRHI::GetVkBufferCreateInfo(Desc.Size, VulkanRHI::GetVkBufferUsage(Desc.Usage));

    if (Desc.Lifetime == ERHIResourceLifetime::Transient)
    {
        VkResult Result = VulkanRHI::vkCreateBuffer(Device.GetVkDevice(), &CreateInfo, nullptr, &Buffer);
        RK_ASSERT(Result == VK_SUCCESS);

        Device.GetTransientResourceAllocator()->AllocBuffer(Buffer, Offset);
    }
    else if (Desc.Lifetime == ERHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent buffer
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = VulkanRHI::ToVmaMemoryUsage(Desc.Memory);

        VkResult Result = vmaCreateBuffer(Device.GetVmaAllocator(), &CreateInfo, &AllocationCreateInfo, &Buffer, &Allocation, nullptr);
        RK_ASSERT(Result == VK_SUCCESS);
    }

    if ((Desc.Usage & ERHIBufferUsage::Uniform) == ERHIBufferUsage::Uniform ||
        (Desc.Usage & ERHIBufferUsage::Storage) == ERHIBufferUsage::Storage)
    {
        m_DescriptorHandle = Device.GetDescriptorManager()->AddStorageBuffer(Buffer, Desc.Size);
    }
}

void FVulkanBuffer::Free()
{
    if (Desc.Lifetime == ERHIResourceLifetime::Transient)
    {
        // Only destroy handle, memory will be freed externally
        Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::Buffer, Buffer, m_DescriptorHandle);
    }
    else if (Desc.Lifetime == ERHIResourceLifetime::Persistent)
    {
        // We own the allocation, free memory
        Device.GetDeferredDeletionQueue()->EnqueueResourceWithAllocation(FDeferredDeletionQueue::EType::Buffer, Buffer, m_DescriptorHandle, Allocation);
    }
}