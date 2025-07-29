#pragma once

#include "Delegate.h"

#include <span>

#include <vulkan/vulkan_core.h>

class FVulkanDevice;
struct FVulkanCommandBuffer;
struct FVulkanCommandBufferPool;

struct FVulkanCommandBufferPayload
{
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkSemaphore> SignalSemaphores;
    std::vector<VkPipelineStageFlags2> WaitStageMasks;
    std::vector<VkPipelineStageFlags2> SignalStageMasks;
    std::vector<FVulkanCommandBuffer*> CommandBuffers;
    uint64_t TimelineSemaphoreValue;

    TDelegate<> PreSubmitCallback;
    TDelegate<> PostExecuteCallback;
};

// Wait -> Execute -> Signal is the allowed forward transition.
// Attempting to regress state (Execute -> Wait) triggers payload completion and starts new payload.
enum class EVulkanCommandBufferContextState 
{ 
    Wait, 
    Execute, 
    Signal 
};

inline bool operator<(EVulkanCommandBufferContextState a, EVulkanCommandBufferContextState b)
{
    return static_cast<int>(a) < static_cast<int>(b);   
}

class FVulkanCommandBufferContext 
{
private:
    FVulkanCommandBufferPool& Pool;

public:
    explicit FVulkanCommandBufferContext(FVulkanCommandBufferPool& InPool);

    FVulkanCommandBuffer* GetCurrentCommandBuffer();
    FVulkanCommandBufferPool* GetCommandBufferPool();
    FVulkanCommandBufferPayload& GetPayload(EVulkanCommandBufferContextState State); 

    void NewPayload();
    void EndPayload();
    void Finalize(std::vector<FVulkanCommandBufferPayload*>& OutPayloads);

    void AddWaitSemaphore(VkSemaphore WaitSemaphore, VkPipelineStageFlags2 WaitStageMask);
    void AddWaitSemaphore(std::span<VkSemaphore> WaitSemaphores, std::span<VkPipelineStageFlags2> WaitStageMasks);
    void AddSignalSemaphore(VkSemaphore ReadySemaphore, VkPipelineStageFlags2 SignalStageMask);
    void AddSignalSemaphore(std::span<VkSemaphore> ReadySemaphores, std::span<VkPipelineStageFlags2> SignalStageMasks);

private:
    std::vector<FVulkanCommandBufferPayload*> Payloads; // todo: std_shared_ptr for memory leak
    EVulkanCommandBufferContextState CurrentState;
    FVulkanCommandBufferPayload* CurrentPayload;
};
