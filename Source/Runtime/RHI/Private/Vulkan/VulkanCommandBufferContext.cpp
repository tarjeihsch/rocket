#include "VulkanCommandBufferContext.h"

#include "VulkanCommandBuffer.h"
#include <Assertion.h>

VulkanCommandBufferContext::VulkanCommandBufferContext(VulkanCommandBufferPool& in_command_buffer_pool)
    : command_buffer_pool(in_command_buffer_pool),
    m_current_state(VulkanCommandBufferContextState::Wait),
    m_current_payload(nullptr)
{
    NewPayload();
}

VulkanCommandBufferPayload& VulkanCommandBufferContext::GetPayload(VulkanCommandBufferContextState state)
{
    if (m_payloads.empty() || state < m_current_state)
    {
        NewPayload();
    }

    m_current_state = state;
    return *m_current_payload;
}

void VulkanCommandBufferContext::NewPayload()
{
    EndPayload();

    m_payloads.push_back(new VulkanCommandBufferPayload());
    m_current_payload = m_payloads.back();
    m_current_state = VulkanCommandBufferContextState::Wait;
}

VulkanCommandBuffer* VulkanCommandBufferContext::GetCurrentCommandBuffer()
{
    VulkanCommandBufferPayload& Payload = GetPayload(VulkanCommandBufferContextState::Wait);

    if (m_current_payload->CommandBuffers.empty())
    {
        VulkanCommandBuffer* CommandBuffer = command_buffer_pool.Acquire();
        Payload.CommandBuffers.push_back(CommandBuffer);
        CommandBuffer->Begin();
    }

    return m_current_payload->CommandBuffers.back();
}

void VulkanCommandBufferContext::EndPayload()
{
    if (m_current_payload && !m_current_payload->CommandBuffers.empty())
    {
        m_current_payload->CommandBuffers.back()->End();
    }
}

void VulkanCommandBufferContext::Finalize(std::vector<VulkanCommandBufferPayload*>& payloads)
{
    EndPayload();

    payloads = std::move(m_payloads);

    m_payloads.clear();
    m_current_payload = nullptr;

    //NewPayload();
}

void VulkanCommandBufferContext::AddWaitSemaphore(VkSemaphore wait_semaphore, VkPipelineStageFlags2 wait_stage_mask)
{
    AddWaitSemaphore(std::span(&wait_semaphore, 1), std::span(&wait_stage_mask, 1));
}

void VulkanCommandBufferContext::AddWaitSemaphore(std::span<VkSemaphore> wait_semaphores, std::span<VkPipelineStageFlags2> wait_stage_masks)
{
    VulkanCommandBufferPayload& Payload = GetPayload(VulkanCommandBufferContextState::Wait);
    Payload.WaitSemaphores.insert(Payload.WaitSemaphores.end(), wait_semaphores.begin(), wait_semaphores.end());
    Payload.WaitStageMasks.insert(Payload.WaitStageMasks.end(), wait_stage_masks.begin(), wait_stage_masks.end());
}

void VulkanCommandBufferContext::AddSignalSemaphore(VkSemaphore ready_semaphore, VkPipelineStageFlags2 signal_stage_mask)
{
    AddSignalSemaphore(std::span(&ready_semaphore, 1), std::span(&signal_stage_mask, 1));
}

void VulkanCommandBufferContext::AddSignalSemaphore(std::span<VkSemaphore> ready_semaphores, std::span<VkPipelineStageFlags2> SignalStageMask)
{
    VulkanCommandBufferPayload& Payload = GetPayload(VulkanCommandBufferContextState::Signal);
    Payload.SignalSemaphores.insert(Payload.SignalSemaphores.end(), ready_semaphores.begin(), ready_semaphores.end());
    Payload.SignalStageMasks.insert(Payload.SignalStageMasks.end(), SignalStageMask.begin(), SignalStageMask.end());
}