#include "VulkanCommandBufferContext.h"

#include "VulkanCommandBuffer.h"

FVulkanCommandBufferContext::FVulkanCommandBufferContext(FVulkanCommandBufferPool& InPool)
    : Pool(InPool),
    CurrentState(EVulkanCommandBufferContextState::Wait),
    CurrentPayload(nullptr)
{
    NewPayload();
}

FVulkanCommandBufferPayload& FVulkanCommandBufferContext::GetPayload(EVulkanCommandBufferContextState State)
{
    if (Payloads.empty() || State < CurrentState)
    {
        NewPayload();
    }

    CurrentState = State;
    return *CurrentPayload;
}

void FVulkanCommandBufferContext::NewPayload()
{
    EndPayload();

    Payloads.push_back(new FVulkanCommandBufferPayload());
    CurrentPayload = Payloads.back();
    CurrentState = EVulkanCommandBufferContextState::Wait;
}

FVulkanCommandBuffer* FVulkanCommandBufferContext::GetCurrentCommandBuffer()
{
    FVulkanCommandBufferPayload& Payload = GetPayload(EVulkanCommandBufferContextState::Wait);

    if (CurrentPayload->CommandBuffers.empty())
    {
        FVulkanCommandBuffer* CommandBuffer = Pool.Acquire();

        Payload.CommandBuffers.push_back(CommandBuffer);

        CommandBuffer->Begin();
    }

    return CurrentPayload->CommandBuffers.back();
}

void FVulkanCommandBufferContext::EndPayload()
{
    if (CurrentPayload && !CurrentPayload->CommandBuffers.empty())
    {
        CurrentPayload->CommandBuffers.back()->End();   
    }
}

void FVulkanCommandBufferContext::Finalize(std::vector<FVulkanCommandBufferPayload*>& OutPayloads)
{
    EndPayload();

    OutPayloads = std::move(Payloads);

    Payloads.clear();
    CurrentPayload = nullptr;

    NewPayload();
}

void FVulkanCommandBufferContext::AddWaitSemaphore(VkSemaphore WaitSemaphore, VkPipelineStageFlags2 WaitStageMask)
{
    AddWaitSemaphore(std::span(&WaitSemaphore, 1), std::span(&WaitStageMask, 1));
}

void FVulkanCommandBufferContext::AddWaitSemaphore(std::span<VkSemaphore> WaitSemaphores, std::span<VkPipelineStageFlags2> WaitStageMasks)
{
    FVulkanCommandBufferPayload& Payload = GetPayload(EVulkanCommandBufferContextState::Wait);
    Payload.WaitSemaphores.insert(Payload.WaitSemaphores.end(), WaitSemaphores.begin(), WaitSemaphores.end());
    Payload.WaitStageMasks.insert(Payload.WaitStageMasks.end(), WaitStageMasks.begin(), WaitStageMasks.end());
}

void FVulkanCommandBufferContext::AddSignalSemaphore(VkSemaphore ReadySemaphore, VkPipelineStageFlags2 SignalStageMask)
{
    AddSignalSemaphore(std::span(&ReadySemaphore, 1), std::span(&SignalStageMask, 1));
}

void FVulkanCommandBufferContext::AddSignalSemaphore(std::span<VkSemaphore> ReadySemaphores, std::span<VkPipelineStageFlags2> SignalStageMask)
{
    FVulkanCommandBufferPayload& Payload = GetPayload(EVulkanCommandBufferContextState::Signal);
    Payload.SignalSemaphores.insert(Payload.SignalSemaphores.end(), ReadySemaphores.begin(), ReadySemaphores.end());
    Payload.SignalStageMasks.insert(Payload.SignalStageMasks.end(), SignalStageMask.begin(), SignalStageMask.end());
}
