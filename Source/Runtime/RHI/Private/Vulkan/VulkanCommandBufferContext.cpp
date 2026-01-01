#include "VulkanCommandBufferContext.h"

#include "VulkanCommandBuffer.h"
#include <Assertion.h>

VulkanCommandBufferContext::VulkanCommandBufferContext(FVulkanCommandBufferPool& in_command_buffer_pool)
    : command_buffer_pool(in_command_buffer_pool),
    m_current_state(VulkanCommandBufferContextState::Wait),
    m_current_payload(nullptr)
{
    new_payload();
}

VulkanCommandBufferPayload& VulkanCommandBufferContext::get_payload(VulkanCommandBufferContextState state)
{
    if (m_payloads.empty() || state < m_current_state)
    {
        new_payload();
    }

    m_current_state = state;
    return *m_current_payload;
}

void VulkanCommandBufferContext::new_payload()
{
    end_payload();

    m_payloads.push_back(new VulkanCommandBufferPayload());
    m_current_payload = m_payloads.back();
    m_current_state = VulkanCommandBufferContextState::Wait;
}

FVulkanCommandBuffer* VulkanCommandBufferContext::get_current_command_buffer()
{
    VulkanCommandBufferPayload& Payload = get_payload(VulkanCommandBufferContextState::Wait);

    if (m_current_payload->CommandBuffers.empty())
    {
        FVulkanCommandBuffer* CommandBuffer = command_buffer_pool.Acquire();
        Payload.CommandBuffers.push_back(CommandBuffer);
        CommandBuffer->Begin();
    }

    return m_current_payload->CommandBuffers.back();
}

void VulkanCommandBufferContext::end_payload()
{
    if (m_current_payload && !m_current_payload->CommandBuffers.empty())
    {
        m_current_payload->CommandBuffers.back()->End();
    }
}

void VulkanCommandBufferContext::finalize(std::vector<VulkanCommandBufferPayload*>& payloads)
{
    end_payload();

    payloads = std::move(m_payloads);

    m_payloads.clear();
    m_current_payload = nullptr;

    new_payload();
}

void VulkanCommandBufferContext::add_wait_semaphore(VkSemaphore wait_semaphore, VkPipelineStageFlags2 wait_stage_mask)
{
    add_wait_semaphore(std::span(&wait_semaphore, 1), std::span(&wait_stage_mask, 1));
}

void VulkanCommandBufferContext::add_wait_semaphore(std::span<VkSemaphore> wait_semaphores, std::span<VkPipelineStageFlags2> wait_stage_masks)
{
    VulkanCommandBufferPayload& Payload = get_payload(VulkanCommandBufferContextState::Wait);
    Payload.WaitSemaphores.insert(Payload.WaitSemaphores.end(), wait_semaphores.begin(), wait_semaphores.end());
    Payload.WaitStageMasks.insert(Payload.WaitStageMasks.end(), wait_stage_masks.begin(), wait_stage_masks.end());
}

void VulkanCommandBufferContext::add_signal_semaphore(VkSemaphore ready_semaphore, VkPipelineStageFlags2 signal_stage_mask)
{
    add_signal_semaphore(std::span(&ready_semaphore, 1), std::span(&signal_stage_mask, 1));
}

void VulkanCommandBufferContext::add_signal_semaphore(std::span<VkSemaphore> ready_semaphores, std::span<VkPipelineStageFlags2> SignalStageMask)
{
    VulkanCommandBufferPayload& Payload = get_payload(VulkanCommandBufferContextState::Signal);
    Payload.SignalSemaphores.insert(Payload.SignalSemaphores.end(), ready_semaphores.begin(), ready_semaphores.end());
    Payload.SignalStageMasks.insert(Payload.SignalStageMasks.end(), SignalStageMask.begin(), SignalStageMask.end());
}