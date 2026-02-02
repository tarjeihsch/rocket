#include "VulkanQueue.h"

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanDevice.h"
#include "Assertion.h"
#include "VulkanCommandListContext.h"

VulkanQueue::VulkanQueue(VulkanDevice& InDevice, VkQueueFlags InQueueFlags, bool InPresent, uint32_t InQueueFamilyIndex, uint32_t InQueueIndex)
    : Device(InDevice),
    QueueFlags(InQueueFlags),
    Present(InPresent), 
    QueueFamilyIndex(InQueueFamilyIndex),
    QueueIndex(InQueueIndex),
    NextTimelineSemaphoreValue(1)
{
    Init();
}

VulkanQueue::~VulkanQueue()
{
    Shutdown();
}

void VulkanQueue::Submit(RHICommandList& CmdList)
{
	VulkanCommandListContext CmdListCtx(Device);
    RHICommandListExecutor::Execute(CmdListCtx, std::move(CmdList.Flush()));
	
    std::vector<VulkanCommandBufferPayload*> Payloads;
    CmdListCtx.Finalize(Payloads);

    for (VulkanCommandBufferPayload* payload : Payloads)
    {
        std::vector<VkSemaphoreSubmitInfo> WaitSemaphoreSubmitInfos;
        for (uint32_t Index = 0; Index < payload->WaitSemaphores.size(); ++Index)
        {
            VkSemaphoreSubmitInfo WaitSemaphoreSubmitInfo = {};
            WaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            WaitSemaphoreSubmitInfo.semaphore = payload->WaitSemaphores[Index];
            WaitSemaphoreSubmitInfo.stageMask = payload->WaitStageMasks[Index];
            WaitSemaphoreSubmitInfos.push_back(WaitSemaphoreSubmitInfo);
        }

        std::vector<VkSemaphoreSubmitInfo> SignalSemaphoreSubmitInfos;
        for (uint32_t Index = 0; Index < payload->SignalSemaphores.size(); ++Index)
        {
            VkSemaphoreSubmitInfo ReadySemaphoreSubmitInfo = {};
            ReadySemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            ReadySemaphoreSubmitInfo.semaphore = payload->SignalSemaphores[Index];
            ReadySemaphoreSubmitInfo.stageMask = payload->SignalStageMasks[Index];
            SignalSemaphoreSubmitInfos.push_back(ReadySemaphoreSubmitInfo);
        }

        std::vector<VkCommandBufferSubmitInfo> CommandBufferSubmitInfos;
        for (VulkanCommandBuffer *CommandBuffer : payload->CommandBuffers)
        {
            VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
            CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            CommandBufferSubmitInfo.commandBuffer = CommandBuffer->GetVkCommandBuffer();
            CommandBufferSubmitInfos.push_back(CommandBufferSubmitInfo);
            CommandBuffer->Submit();
        }

        uint64_t TimelineValue = NextTimelineSemaphoreValue++;
        payload->TimelineSemaphoreValue = TimelineValue;
        LastSubmittedTimelineValue = TimelineValue;

        VkSemaphoreSubmitInfo TimelineSemaphoreSubmitInfo = {};
        TimelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        TimelineSemaphoreSubmitInfo.semaphore = TimelineSemaphore;
        TimelineSemaphoreSubmitInfo.value = TimelineValue;
        TimelineSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;
        SignalSemaphoreSubmitInfos.push_back(TimelineSemaphoreSubmitInfo);

        if (payload->PreSubmitCallback.Size())
        {
            payload->PreSubmitCallback.Broadcast();
        }

        VkSubmitInfo2 SubmitInfo = {};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        SubmitInfo.waitSemaphoreInfoCount = WaitSemaphoreSubmitInfos.size();
        SubmitInfo.pWaitSemaphoreInfos = WaitSemaphoreSubmitInfos.data();
        SubmitInfo.signalSemaphoreInfoCount = SignalSemaphoreSubmitInfos.size();
        SubmitInfo.pSignalSemaphoreInfos = SignalSemaphoreSubmitInfos.data();
        SubmitInfo.commandBufferInfoCount = CommandBufferSubmitInfos.size();
        SubmitInfo.pCommandBufferInfos = CommandBufferSubmitInfos.data();

        VkResult Result = VulkanRHI::vkQueueSubmit2(Handle, 1, &SubmitInfo, nullptr);
        ensure(Result == VK_SUCCESS, "Failed to submit command buffer to graphics queue.");
        LastSubmittedTimelineValue = TimelineValue;

        if (payload->PostSubmitCallback.Size())
        {
            payload->PostSubmitCallback.Broadcast();
        }

        SubmissionQueue.push(payload);
    }
}

void VulkanQueue::WaitForTimelineValue(uint64_t Value)
{
    VkSemaphoreWaitInfo WaitInfo = {};
    WaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    WaitInfo.semaphoreCount = 1;
    WaitInfo.pSemaphores = &TimelineSemaphore;
    WaitInfo.pValues = &Value;

    VkResult Result = VulkanRHI::vkWaitSemaphores(Device.GetVkDevice(), &WaitInfo, UINT64_MAX);
    ensure(Result == VK_SUCCESS);

    Result = VulkanRHI::vkGetSemaphoreCounterValue(Device.GetVkDevice(), TimelineSemaphore, &LastCompletedTimelineValue);
    ensure(Result == VK_SUCCESS);
}

void VulkanQueue::Retire()
{
    VkResult Result = VulkanRHI::vkGetSemaphoreCounterValue(Device.GetVkDevice(), TimelineSemaphore, &LastCompletedTimelineValue);
    ensure(Result == VK_SUCCESS);

    while (!SubmissionQueue.empty())
    {
        VulkanCommandBufferPayload* Payload = SubmissionQueue.front();
        if (Payload->TimelineSemaphoreValue > LastCompletedTimelineValue)
        {
            break;
        }

        SubmissionQueue.pop();

        for (VulkanCommandBuffer* CommandBuffer : Payload->CommandBuffers)
        {
            CommandBuffer->Retire();
            CommandBufferPool->Recycle(CommandBuffer);
        }

        delete Payload;
    }
}

VulkanCommandBufferPool* VulkanQueue::GetCommandBufferPool() const
{
    return CommandBufferPool;
}

void VulkanQueue::Init()
{
    VulkanRHI::vkGetDeviceQueue(Device.GetVkDevice(), QueueFamilyIndex, 0, &Handle);

    VkSemaphoreTypeCreateInfo TimelineSemaphoreCreateInfo = {};
    TimelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    TimelineSemaphoreCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    TimelineSemaphoreCreateInfo.initialValue  = 0;

    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    SemaphoreCreateInfo.pNext = &TimelineSemaphoreCreateInfo;
    VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &TimelineSemaphore);

    CommandBufferPool = new VulkanCommandBufferPool(Device, *this);
}

void VulkanQueue::Shutdown()
{
    while (!SubmissionQueue.empty())
    {
        VulkanCommandBufferPayload* Payload = SubmissionQueue.front();
        SubmissionQueue.pop();
        delete Payload;
    }

    delete CommandBufferPool;

    VulkanRHI::vkDestroySemaphore(Device.GetVkDevice(), TimelineSemaphore, nullptr);
}
