#pragma once

#include "VulkanCommandBuffer.h"
#include "VulkanCommandList.h"

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <vector>

class FVulkanCommandListContext;
class FVulkanDevice;
struct FVulkanTexture;
class FVulkanQueue;
class FVulkanPipelineManager;
class VulkanDescriptorManager;
class VulkanResourceManager;

struct FTransientResourceAllocator
{
    FTransientResourceAllocator(FVulkanDevice& InDevice, VkDeviceSize InSize)
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

    FVulkanDevice& Device;
};

class FDeferredDeletionQueue
{
public:
    FDeferredDeletionQueue(FVulkanDevice& InDevice);
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
        ShaderModule,
    };

    template <typename T>
    inline void EnqueueResource(EType Type, T Handle, DescriptorHandle BindingIndex)
    {
        static_assert(sizeof(T) <= sizeof(uint64_t), "Vulkan resource handle type size too large.");
        EnqueueGenericResource(Type, (uint64_t)Handle, BindingIndex, nullptr);
    }

    template <typename T>
    inline void EnqueueResourceWithAllocation(EType Type, T Handle, DescriptorHandle BindingIndex, VmaAllocation Allocation)
    {
        static_assert(sizeof(T) <= sizeof(uint64_t), "Vulkan resource handle type size too large.");
        EnqueueGenericResource(Type, (uint64_t)Handle, BindingIndex, Allocation);
    }

    void EnqueueGenericResource(EType Type, uint64_t Handle, DescriptorHandle BindingIndex, VmaAllocation Allocation);

    void ReleaseExpiredResources(uint64_t CompletedFrameNumber);

private:
    struct FEntry
    {
        EType StructureType;
        uint64_t FrameNumber;
        uint64_t Handle;
        DescriptorHandle m_DescriptorHandle;
        VmaAllocation Allocation;
    };

    std::vector<FEntry> Entries;

    FVulkanDevice& Device;
};

class FVulkanDevice
{
public:
    FVulkanDevice(VkPhysicalDevice InPhysicalDevice);
    ~FVulkanDevice();

    void WaitUntilIdle() const;

    FVulkanQueue* GetQueue(VkQueueFlags Include, VkQueueFlags Exclude) const;
    FVulkanQueue* GetGraphicsQueue() const;
    FVulkanQueue* GetComputeQueue() const;
    FVulkanQueue* GetTransferQueue() const;
    FVulkanQueue* GetPresentQueue() const;

    inline VkDevice GetVkDevice() const;
    inline VkPhysicalDevice GetVkPhysicalDevice() const;
    inline VmaAllocator GetVmaAllocator() const;
    inline VkDebugUtilsMessengerEXT GetVkDebugUtilsMessenger() const;

    inline FVulkanPipelineManager* GetPipelineManager() const;
    inline VulkanDescriptorManager* GetDescriptorManager() const;

    inline FTransientResourceAllocator* GetTransientResourceAllocator() const;
    inline FVulkanCommandListContext* GetCommandListContext() const;
    inline FVulkanCommandListContext* GetImmediateCommandListContext() const;
    inline FDeferredDeletionQueue* GetDeferredDeletionQueue() const;

    void GC();

private:
    void Initialize();
    void Shutdown();

private:
    VkDevice Device;
    VmaAllocator Allocator;
    VkDebugUtilsMessengerEXT DebugUtilsMessenger;

    std::vector<FVulkanQueue*> Queues;

    FVulkanPipelineManager* PipelineManager;

    VulkanDescriptorManager* DescriptorManager;

    FDeferredDeletionQueue* DeferredDeletionQueue;

    FVulkanCommandListContext* CommandListContext;

    FVulkanCommandListContext* ImmediateCommandListContext;

    FTransientResourceAllocator* TransientResourceAllocator;

private:
    VkPhysicalDevice PhysicalDevice;
};

inline VkDevice FVulkanDevice::GetVkDevice() const
{
    return Device;
}

inline VkPhysicalDevice FVulkanDevice::GetVkPhysicalDevice() const
{
    return PhysicalDevice;
}

inline VmaAllocator FVulkanDevice::GetVmaAllocator() const
{
    return Allocator;
}

inline VkDebugUtilsMessengerEXT FVulkanDevice::GetVkDebugUtilsMessenger() const
{
    return DebugUtilsMessenger;
}

inline FVulkanPipelineManager* FVulkanDevice::GetPipelineManager() const
{
    return PipelineManager;
}

inline VulkanDescriptorManager* FVulkanDevice::GetDescriptorManager() const
{
    return DescriptorManager;
}

inline FTransientResourceAllocator *FVulkanDevice::GetTransientResourceAllocator() const
{
    return TransientResourceAllocator;
}

inline FVulkanCommandListContext *FVulkanDevice::GetCommandListContext() const
{
    return CommandListContext;
}

inline FDeferredDeletionQueue *FVulkanDevice::GetDeferredDeletionQueue() const
{
    return DeferredDeletionQueue;
}