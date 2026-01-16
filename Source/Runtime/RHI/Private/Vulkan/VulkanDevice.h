#pragma once

#include "VulkanDescriptorManager.h"

#include <vector>
#include <vma/vk_mem_alloc.h>

class VulkanCommandListContext;
class VulkanDevice;
struct VulkanTexture;
class VulkanQueue;
class VulkanPipelineManager;
class VulkanDescriptorManager;
class VulkanResourceManager;

struct FTransientResourceAllocator
{
    FTransientResourceAllocator(VulkanDevice& InDevice, VkDeviceSize InSize)
        : Device(InDevice), Size(InSize)
    {
        Init();
    }

    ~FTransientResourceAllocator()
    {
        Shutdown();
    }

    void Reset();

    void AllocImage(VkImage Image, uint64_t& OutOffset);
    void AllocBuffer(VkBuffer Buffer, uint64_t& OutOffset);

    void* GetMappedBasePtr() const;

    inline size_t GetSize() const;
    inline size_t GetOffset() const;

private:
    void Init();
    void Shutdown();

private:
    VmaVirtualBlock VirtualMemory = nullptr;
    VkDeviceMemory PhysicalMemory = nullptr;
    VkDeviceSize Offset = 0;
    VkDeviceSize Size = 0;
    void* MappedBasePtr = nullptr;

    uint64_t TotalAllocations;
    uint64_t CurrentAllocations;

    VulkanDevice& Device;
};

inline size_t FTransientResourceAllocator::GetSize() const
{
    return Size;
}

inline size_t FTransientResourceAllocator::GetOffset() const
{
    return Offset;
}

class FDeferredDeletionQueue
{
public:
    FDeferredDeletionQueue(VulkanDevice& InDevice);
    ~FDeferredDeletionQueue();

    enum class EType
    {
        Buffer,
        BufferView,
        Image,
        ImageView,
        Pipeline,
        PipelineLayout,
        DescriptorSetLayout,
        Sampler,
        Semaphore,
        ShaderModule
    };

    template <typename T>
    inline void enqueue_resource(EType Type, T Handle, BindlessHandle BindingIndex)
    {
        static_assert(sizeof(T) <= sizeof(uint64_t), "Vulkan resource handle type size too large.");
        EnqueueGenericResource(Type, (uint64_t)Handle, BindingIndex, nullptr);
    }

    template <typename T>
    inline void enqueue_resource_with_allocation(EType Type, T Handle, BindlessHandle BindingIndex, VmaAllocation Allocation)
    {
        static_assert(sizeof(T) <= sizeof(uint64_t), "Vulkan resource handle type size too large.");
        EnqueueGenericResource(Type, (uint64_t)Handle, BindingIndex, Allocation);
    }

    void EnqueueGenericResource(EType Type, uint64_t Handle, BindlessHandle BindingIndex, VmaAllocation Allocation);
    void ReleaseExpiredResources(uint64_t CompletedFrameNumber);

private:
    struct FEntry
    {
        EType StructureType;
        uint64_t FrameNumber;
        uint64_t Handle;
        BindlessHandle m_DescriptorHandle;
        VmaAllocation Allocation;
    };

    std::vector<FEntry> Entries;

    VulkanDevice& Device;
};

class VulkanDevice
{
public:
    VulkanDevice(VkPhysicalDevice InPhysicalDevice);
    ~VulkanDevice();

    void WaitUntilIdle() const;

    uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);

    VulkanQueue* GetQueue(VkQueueFlags Include, VkQueueFlags Exclude) const;
    VulkanQueue* GetGraphicsQueue() const;
    VulkanQueue* GetComputeQueue() const;
    VulkanQueue* GetTransferQueue() const;
    VulkanQueue* GetPresentQueue() const;

    inline VkDevice GetVkDevice() const;
    inline VkPhysicalDevice GetVkPhysicalDevice() const;
    inline VmaAllocator get_vma_allocator() const;
    inline VkDebugUtilsMessengerEXT GetVkDebugUtilsMessenger() const;

    inline VulkanPipelineManager* GetPipelineManager() const;
    inline VulkanDescriptorManager* GetBindlessDescriptorManager() const;
    inline FTransientResourceAllocator* GetTransientMemoryAllocator() const;
    inline FDeferredDeletionQueue* get_deferred_deletion_queue() const;

    void GC();

private:
    void Initialize();
    void Shutdown();

private:
    VkDevice Device;
    VmaAllocator Allocator;
    VkDebugUtilsMessengerEXT DebugUtilsMessenger;

    std::vector<VulkanQueue*> Queues;

    VulkanPipelineManager* PipelineManager;
    VulkanDescriptorManager* DescriptorManager;
    FDeferredDeletionQueue* DeferredDeletionQueue;
    VulkanCommandListContext* CommandListContext;
    VulkanCommandListContext* ImmediateCommandListContext;
    FTransientResourceAllocator* TransientResourceAllocator;

private:
    VkPhysicalDevice PhysicalDevice;
};

inline VkDevice VulkanDevice::GetVkDevice() const
{
    return Device;
}

inline VkPhysicalDevice VulkanDevice::GetVkPhysicalDevice() const
{
    return PhysicalDevice;
}

inline VmaAllocator VulkanDevice::get_vma_allocator() const
{
    return Allocator;
}

inline VkDebugUtilsMessengerEXT VulkanDevice::GetVkDebugUtilsMessenger() const
{
    return DebugUtilsMessenger;
}

inline VulkanPipelineManager* VulkanDevice::GetPipelineManager() const
{
    return PipelineManager;
}

inline VulkanDescriptorManager* VulkanDevice::GetBindlessDescriptorManager() const
{
    return DescriptorManager;
}

inline FTransientResourceAllocator *VulkanDevice::GetTransientMemoryAllocator() const
{
    return TransientResourceAllocator;
}

inline FDeferredDeletionQueue *VulkanDevice::get_deferred_deletion_queue() const
{
    return DeferredDeletionQueue;
}