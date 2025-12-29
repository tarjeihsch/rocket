#pragma once

#include <span>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "Events/Delegate.h"

class RHISwapchain;
class VulkanSwapchain;
struct RHISemaphore;
class FVulkanDevice;
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
    FVulkanCommandBufferPool& pool;

public:
    explicit VulkanCommandBufferContext(FVulkanCommandBufferPool& InPool);

    FVulkanCommandBuffer* get_current_command_buffer();
    VulkanCommandBufferPayload& get_payload(VulkanCommandBufferContextState State);

    void new_payload();
    void end_payload();

    void add_pre_submit_event(std::function<void()>&& callback);
    void add_post_submit_event(std::function<void()>&& callback);

    void submit(std::vector<VulkanCommandBufferPayload*>& OutPayloads);

    void add_wait_semaphore(VkSemaphore WaitSemaphore, VkPipelineStageFlags2 WaitStageMask);
    void add_wait_semaphore(std::span<VkSemaphore> WaitSemaphores, std::span<VkPipelineStageFlags2> WaitStageMasks);
    void add_signal_semaphore(VkSemaphore ReadySemaphore, VkPipelineStageFlags2 SignalStageMask);
    void add_signal_semaphore(std::span<VkSemaphore> ReadySemaphores, std::span<VkPipelineStageFlags2> SignalStageMasks);

private:
    std::vector<VulkanCommandBufferPayload*> m_payloads;

    VulkanCommandBufferPayload* m_current_payload;
    VulkanCommandBufferContextState m_current_state;
};
