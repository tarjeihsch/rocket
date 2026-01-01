#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Events/Delegate.h"

class RHISwapchain;
class VulkanSwapchain;
struct RHISemaphore;
class VulkanDevice;
struct FVulkanCommandBuffer;
struct FVulkanCommandBufferPool;

struct VulkanCommandBufferPayload
{
    std::vector<VkSemaphore> WaitSemaphores;
    std::vector<VkSemaphore> SignalSemaphores;
    std::vector<VkPipelineStageFlags2> WaitStageMasks;
    std::vector<VkPipelineStageFlags2> SignalStageMasks;
    std::vector<FVulkanCommandBuffer*> CommandBuffers;
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

inline bool operator<(VulkanCommandBufferContextState a, VulkanCommandBufferContextState b)
{
    return static_cast<int>(a) < static_cast<int>(b);   
}

class VulkanCommandBufferContext
{
private:
    FVulkanCommandBufferPool& command_buffer_pool;

public:
    explicit VulkanCommandBufferContext(FVulkanCommandBufferPool& in_command_buffer_pool);

    FVulkanCommandBuffer* get_current_command_buffer();
    VulkanCommandBufferPayload& get_payload(VulkanCommandBufferContextState state);

    void new_payload();
    void end_payload();

    void finalize(std::vector<VulkanCommandBufferPayload*>& payloads);

    void add_wait_semaphore(VkSemaphore wait_semaphore, VkPipelineStageFlags2 wait_stage_mask);
    void add_wait_semaphore(std::span<VkSemaphore> wait_semaphores, std::span<VkPipelineStageFlags2> wait_stage_masks);
    void add_signal_semaphore(VkSemaphore ready_semaphore, VkPipelineStageFlags2 signal_stage_mask);
    void add_signal_semaphore(std::span<VkSemaphore> ready_semaphores, std::span<VkPipelineStageFlags2> signal_stage_masks);

private:
    std::vector<VulkanCommandBufferPayload*> m_payloads;

    VulkanCommandBufferPayload* m_current_payload;
    VulkanCommandBufferContextState m_current_state;
};
