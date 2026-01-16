#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "VulkanBuffer.h"
#include "VulkanTexture.h"
#include "Events/Delegate.h"

class RHISwapchain;
class VulkanSwapchain;
struct RHISemaphore;
class VulkanDevice;
struct VulkanCommandBuffer;
struct VulkanCommandBufferPool;

struct VulkanCommandBufferPayload
{
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkSemaphore> SignalSemaphores;
    std::vector<VkPipelineStageFlags2> WaitStageMasks;
    std::vector<VkPipelineStageFlags2> SignalStageMasks;
    std::vector<VulkanCommandBuffer*> CommandBuffers;
    uint64_t TimelineSemaphoreValue;

    TDelegate<> PreSubmitCallback;
    TDelegate<> PostSubmitCallback;
};

// Wait -> Execute -> Signal is the allowed forward transition.
// Attempting to regress state (Execute -> Wait) triggers payload completion and starts new payload.
enum class VulkanCommandBufferContextState
{ 
    Wait, 
    Execute, 
    Signal 
};

inline bool operator<(VulkanCommandBufferContextState lhs, VulkanCommandBufferContextState rhs)
{
    return static_cast<int32_t>(lhs) < static_cast<int32_t>(rhs);
}

class VulkanCommandBufferContext
{
private:
    VulkanCommandBufferPool& command_buffer_pool;

public:
    explicit VulkanCommandBufferContext(VulkanCommandBufferPool& in_command_buffer_pool);

    VulkanCommandBuffer* GetCurrentCommandBuffer();
    VulkanCommandBufferPayload& GetPayload(VulkanCommandBufferContextState state);

    void NewPayload();
    void EndPayload();

    void Finalize(std::vector<VulkanCommandBufferPayload*>& payloads);

    void AddWaitSemaphore(VkSemaphore wait_semaphore, VkPipelineStageFlags2 wait_stage_mask);
    void AddWaitSemaphore(std::span<VkSemaphore> wait_semaphores, std::span<VkPipelineStageFlags2> wait_stage_masks);
    void AddSignalSemaphore(VkSemaphore ready_semaphore, VkPipelineStageFlags2 signal_stage_mask);
    void AddSignalSemaphore(std::span<VkSemaphore> ready_semaphores, std::span<VkPipelineStageFlags2> signal_stage_masks);

private:
    std::vector<VulkanCommandBufferPayload*> m_payloads;

    VulkanCommandBufferPayload* m_current_payload;
    VulkanCommandBufferContextState m_current_state;
};
