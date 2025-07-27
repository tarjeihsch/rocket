#pragma once

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

#include <memory>
#include <vector>

class FVulkanQueue;
class VulkanPipelineManager;
class VulkanDescriptorManager;
class VulkanResourceManager;

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

private:
    void Initialize();
    void Shutdown();

private:
    VkDevice Device;
    VmaAllocator Allocator;
    VkDebugUtilsMessengerEXT DebugUtilsMessenger;
    std::vector<std::unique_ptr<FVulkanQueue>> Queues;

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