#pragma once

#include <functional>
#include <queue>
#include <vulkan/vulkan_core.h>

class RHICommandListContext;
class RHICommandList;
class VulkanDevice;
class VulkanCommandBufferPool;
class VulkanCommandBufferContext;
class VulkanCommandBuffer;
struct VulkanCommandBufferPayload;

class VulkanQueue
{
private:
    VulkanDevice& Device;
    VkQueueFlags QueueFlags;

public:
    VulkanQueue(VulkanDevice& InDevice, VkQueueFlags InQueueFlags, bool InPresent, uint32_t InQueueFamilyIndex, uint32_t InQueueIndex);
     ~VulkanQueue();

    void Submit(RHICommandList& CmdList);
    void WaitForTimelineValue(uint64_t TimelineValue);
    void Retire();

    inline uint64_t GetLastCompletedTimelineValue() const { return LastCompletedTimelineValue; }
    inline uint64_t GetLastSubmittedTimelineValue() const { return LastSubmittedTimelineValue; }
    
    inline VkQueue GetHandle() const;
    inline VkQueueFlags GetQueueFlags() const;
    inline bool HasPresent() const;
    inline uint32_t GetQueueFamilyIndex() const;
    inline uint32_t GetQueueIndex() const;

    inline VkSemaphore GetTimelineSemaphore() const;
    inline uint64_t GetTimelineSemaphoreValue() const;

    VulkanCommandBufferPool* GetCommandBufferPool() const;

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
	uint64_t LastSubmittedTimelineValue = 0;
	uint64_t LastCompletedTimelineValue = 0;

    VulkanCommandBufferPool* CommandBufferPool;
    std::queue<VulkanCommandBufferPayload*> SubmissionQueue;
};

inline VkQueue VulkanQueue::GetHandle() const
{
    return Handle;
}

inline VkQueueFlags VulkanQueue::GetQueueFlags() const
{
    return QueueFlags;
}

inline bool VulkanQueue::HasPresent() const
{
    return Present;
}

inline uint32_t VulkanQueue::GetQueueFamilyIndex() const
{
    return QueueFamilyIndex;
}

inline uint32_t VulkanQueue::GetQueueIndex() const
{
    return QueueIndex;
}

uint64_t VulkanQueue::GetTimelineSemaphoreValue() const
{
    return NextTimelineSemaphoreValue;
}

VkSemaphore VulkanQueue::GetTimelineSemaphore() const
{
    return TimelineSemaphore;
}