#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanCommandBufferPool;
class VulkanQueue;
class VulkanDevice;

enum class EVulkanCommandBufferType : uint8_t
{
    Primary,
    Secondary,
};

enum class VulkanCommandBufferState : uint8_t
{
    NotAllocated,
    ReadyForBegin,
    IsInsideBegin,
    HasEnded,
    Submitted,
    NeedReset
};

class VulkanCommandBuffer
{
private:
    VulkanCommandBufferPool& Pool;

public:
    VulkanCommandBuffer(VulkanCommandBufferPool& InPool);
    ~VulkanCommandBuffer();

    void Begin();
    void End();

    void Submit();
    void Retire();
    void Reset();

    inline VulkanCommandBufferState GetState() const;
    inline VkCommandBuffer GetVkCommandBuffer() const;
    inline double GetSubmissionTime() const;

private:
    void Alloc();
    void Free();

    inline uint32_t GetPoolID() const;
    inline void SetPoolID(const uint32_t InPoolID);

private:
    VulkanCommandBufferState State;
    VkCommandBuffer Handle;
    double SubmissionTime;
    uint32_t PoolID;

    friend class VulkanCommandBufferPool;
};

class VulkanCommandBufferPool
{
private:
    VulkanDevice& Device;
    VulkanQueue& Queue;

public:
    VulkanCommandBufferPool(VulkanDevice& InDevice, VulkanQueue& InQueue);
    ~VulkanCommandBufferPool();

    VulkanCommandBuffer* Acquire();
    void Recycle(VulkanCommandBuffer* CommandBuffer);

    inline VkCommandPool GetHandle() const;

private:
    void Initialize();
    void Shutdown();

private:
    VkCommandPool Handle;

    std::vector<VulkanCommandBuffer*> FreeCommandBuffers;
    std::vector<VulkanCommandBuffer*> UsedCommandBuffers;
};

VulkanCommandBufferState VulkanCommandBuffer::GetState() const
{
    return State;
}

VkCommandBuffer VulkanCommandBuffer::GetVkCommandBuffer() const
{
    return Handle;
}

double VulkanCommandBuffer::GetSubmissionTime() const
{
    return SubmissionTime;
}

inline uint32_t VulkanCommandBuffer::GetPoolID() const
{
    return PoolID;
}

inline void VulkanCommandBuffer::SetPoolID(const uint32_t InPoolID)
{
    PoolID = InPoolID;
}

VkCommandPool VulkanCommandBufferPool::GetHandle() const
{
    return Handle;
}
