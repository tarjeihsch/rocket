#pragma once

#include "Common/RHICommandList.h"

#include <functional>
#include <memory>
#include <queue>
#include <vulkan/vulkan_core.h>

class IRHICommandListContext;
class FRHICommandList;
class FVulkanDevice;
struct FVulkanCommandBufferPayload;
class FVulkanCommandBufferPool;
class FVulkanCommandBufferContext;
class FVulkanCommandBuffer;

class FVulkanQueue
{
private:
    FVulkanDevice& Device;
    VkQueueFlags QueueFlags;

public:
    FVulkanQueue(FVulkanDevice& InDevice, VkQueueFlags InQueueFlags, bool InPresent, uint32_t InQueueFamilyIndex, uint32_t InQueueIndex);
     ~FVulkanQueue();

    void Wait();
    void Submit();
    void ImmediateSubmit(std::function<void(FRHICommandList& CmdList)> InCallback);

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
    std::queue<FVulkanCommandBufferPayload*> SubmissionQueue;
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