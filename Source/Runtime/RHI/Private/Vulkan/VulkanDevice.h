#pragma once

#include "VulkanFrameGraph.h"

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

#include <memory>
#include <vector>

class FVulkanQueue;
class VulkanPipelineManager;
class VulkanDescriptorManager;
class VulkanResourceManager;

struct FFGScratchAllocator
{
    FVulkanDevice& Device;

    FFGScratchAllocator(FVulkanDevice& InDevice, VkDeviceSize InSize)
        : Device(InDevice), Size(InSize)
    {
    }

    VmaVirtualBlock VirtualMemory = nullptr;
    VkDeviceMemory PhysicalMemory = nullptr;
    VkDeviceSize Offset = 0;
    VkDeviceSize Size = 0;

    void CreateScratchAllocator();
    void DestroyScratchAllocator();
    void ResetScratchAllocator();
};

class FVulkanDevice
{
private:
    VkPhysicalDevice PhysicalDevice;

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

    inline VulkanPipelineManager* GetPipelineManager() const;
    inline VulkanDescriptorManager* GetDescriptorManager() const;
    inline VulkanResourceManager* GetResourceManager() const;

    inline FFGScratchAllocator *GetTextureScratchAllocator() const;
    inline FFGScratchAllocator *GetBufferScratchAllocator() const;

    void GC();

private:
    void Initialize();
    void Shutdown();

private:
    VkDevice Device;
    VmaAllocator Allocator;
    VkDebugUtilsMessengerEXT DebugUtilsMessenger;
    std::vector<std::unique_ptr<FVulkanQueue>> Queues;

    FFGScratchAllocator* TextureScratchAllocator;
    FFGScratchAllocator* BufferScratchAllocator;

    VulkanPipelineManager* PipelineManager;
    VulkanDescriptorManager* DescriptorManager;
    VulkanResourceManager* ResourceManager;
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

inline VulkanPipelineManager *FVulkanDevice::GetPipelineManager() const
{
    return PipelineManager;
}

inline VulkanDescriptorManager *FVulkanDevice::GetDescriptorManager() const
{
    return DescriptorManager;
}

inline VulkanResourceManager *FVulkanDevice::GetResourceManager() const
{
    return ResourceManager;
}

inline FFGScratchAllocator *FVulkanDevice::GetTextureScratchAllocator() const
{
    return TextureScratchAllocator;
}

inline FFGScratchAllocator *FVulkanDevice::GetBufferScratchAllocator() const
{
    return BufferScratchAllocator;
}