#include "VulkanCommandBufferContext.h"

#include "VulkanCommandBuffer.h"
#include <Assertion.h>

VulkanCommandBufferContext::VulkanCommandBufferContext(FVulkanCommandBufferPool& InPool)
    : pool(InPool),
    m_current_state(VulkanCommandBufferContextState::Wait),
    m_current_payload(nullptr)
{
    new_payload();
}

VulkanCommandBufferPayload& VulkanCommandBufferContext::get_payload(VulkanCommandBufferContextState State)
{
    if (m_payloads.empty() || State < m_current_state)
    {
        new_payload();
    }

    m_current_state = State;
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
        FVulkanCommandBuffer* CommandBuffer = pool.Acquire();
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

void VulkanCommandBufferContext::add_pre_submit_event(std::function<void()>&& callback)
{
    if (m_current_payload)
    {
        m_current_payload->PreSubmitCallback.Bind(std::move(callback));
    }
}

void VulkanCommandBufferContext::add_post_submit_event(std::function<void()>&& callback)
{
    if (m_current_payload)
    {
        m_current_payload->PostSubmitCallback.Bind(std::move(callback));
    }
}

void VulkanCommandBufferContext::submit(std::vector<VulkanCommandBufferPayload*>& OutPayloads)
{
    end_payload();

    OutPayloads = std::move(m_payloads);

    m_payloads.clear();
    m_current_payload = nullptr;

    new_payload();
}

void VulkanCommandBufferContext::add_wait_semaphore(VkSemaphore WaitSemaphore, VkPipelineStageFlags2 WaitStageMask)
{
    add_wait_semaphore(std::span(&WaitSemaphore, 1), std::span(&WaitStageMask, 1));
}

void VulkanCommandBufferContext::add_wait_semaphore(std::span<VkSemaphore> WaitSemaphores, std::span<VkPipelineStageFlags2> WaitStageMasks)
{
    VulkanCommandBufferPayload& Payload = get_payload(VulkanCommandBufferContextState::Wait);
    Payload.WaitSemaphores.insert(Payload.WaitSemaphores.end(), WaitSemaphores.begin(), WaitSemaphores.end());
    Payload.WaitStageMasks.insert(Payload.WaitStageMasks.end(), WaitStageMasks.begin(), WaitStageMasks.end());
}

void VulkanCommandBufferContext::add_signal_semaphore(VkSemaphore ReadySemaphore, VkPipelineStageFlags2 SignalStageMask)
{
    add_signal_semaphore(std::span(&ReadySemaphore, 1), std::span(&SignalStageMask, 1));
}

void VulkanCommandBufferContext::add_signal_semaphore(std::span<VkSemaphore> ReadySemaphores, std::span<VkPipelineStageFlags2> SignalStageMask)
{
    VulkanCommandBufferPayload& Payload = get_payload(VulkanCommandBufferContextState::Signal);
    Payload.SignalSemaphores.insert(Payload.SignalSemaphores.end(), ReadySemaphores.begin(), ReadySemaphores.end());
    Payload.SignalStageMasks.insert(Payload.SignalStageMasks.end(), SignalStageMask.begin(), SignalStageMask.end());
}