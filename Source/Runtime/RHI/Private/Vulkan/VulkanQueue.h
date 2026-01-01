#pragma once

#include <functional>
#include <queue>
#include <vulkan/vulkan_core.h>

class RHICommandListContext;
class RHICommandList;
class VulkanDevice;
class FVulkanCommandBufferPool;
class VulkanCommandBufferContext;
class FVulkanCommandBuffer;
struct VulkanCommandBufferPayload;

class FVulkanQueue
{
private:
    VulkanDevice& Device;
    VkQueueFlags QueueFlags;

public:
    FVulkanQueue(VulkanDevice& InDevice, VkQueueFlags InQueueFlags, bool InPresent, uint32_t InQueueFamilyIndex, uint32_t InQueueIndex);
     ~FVulkanQueue();

    void Wait();
    void Submit(const RHICommandList& CmdList);
    void ImmediateSubmit(std::function<void(RHICommandList& CmdList)> InCallback);

    inline VkQueue GetHandle() const;
    inline VkQueueFlags GetQueueFlags() const;
    inline bool HasPresent() const;
    inline uint32_t GetQueueFamilyIndex() const;
    inline uint32_t GetQueueIndex() const;

    inline VkSemaphore GetTimelineSemaphore() const;
    inline uint64_t GetTimelineSemaphoreValue() const;

    FVulkanCommandBufferPool* GetCommandBufferPool() const;

private:
    void Init();
    void Shutdown();

private:
    VkQueue Handle;
    bool Present;
    uint32_t QueueFamilyIndex;
    uint32_t QueueIndex;

    VkSemaphore TimelineSemaphore;
    uint64_t NextTimelineSemaphoreValue;

    FVulkanCommandBufferPool* CommandBufferPool;
    std::queue<VulkanCommandBufferPayload*> SubmissionQueue;
};

inline VkQueue FVulkanQueue::GetHandle() const
{
    return Handle;
}

inline VkQueueFlags FVulkanQueue::GetQueueFlags() const
{
    return QueueFlags;
}

inline bool FVulkanQueue::HasPresent() const
{
    return Present;
}

inline uint32_t FVulkanQueue::GetQueueFamilyIndex() const
{
    return QueueFamilyIndex;
}

inline uint32_t FVulkanQueue::GetQueueIndex() const
{
    return QueueIndex;
}

uint64_t FVulkanQueue::GetTimelineSemaphoreValue() const
{
    return NextTimelineSemaphoreValue;
}

VkSemaphore FVulkanQueue::GetTimelineSemaphore() const
{
    return TimelineSemaphore;
}