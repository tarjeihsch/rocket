#include "VulkanQueue.h"

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanDevice.h"
#include "Assert.h"
#include "VulkanCommandList.h"

#include <cstring>

FVulkanQueue::FVulkanQueue(FVulkanDevice& InDevice, VkQueueFlags InQueueFlags, bool InPresent, uint32_t InQueueFamilyIndex, uint32_t InQueueIndex)
    : Device(InDevice),
    QueueFlags(InQueueFlags),
    Present(InPresent), 
    QueueFamilyIndex(InQueueFamilyIndex),
    QueueIndex(InQueueIndex),
    NextTimelineSemaphoreValue(1)
{
    Init();
}

FVulkanQueue::~FVulkanQueue()
{
    Shutdown();
}

void FVulkanQueue::Submit()
{
    std::vector<FVulkanCommandBufferPayload *> Payloads;
    auto CmdListContext = static_cast<FVulkanCommandListContext *>(GetRHI()->GetCommandListContext());
    CmdListContext->Finalize(Payloads);

    for (FVulkanCommandBufferPayload *Payload : Payloads)
    {
        bool bIsFirst = (Payload == Payloads.front());
        bool bIsLast = (Payload == Payloads.back());

        if (bIsFirst)
        {
            Payload->WaitSemaphores.push_back(GetVulkanRHIMinimal()->GetImageAcquiredSemaphore());
            Payload->WaitStageMasks.push_back(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR); // todo: these are not correct
        }

        if (bIsLast)
        {
            Payload->SignalSemaphores.push_back(GetVulkanRHIMinimal()->GetRenderFinishedSemaphore());
            Payload->SignalStageMasks.push_back(VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR); // todo: these are not correct
        }

        std::vector<VkSemaphoreSubmitInfo> WaitSemaphoreSubmitInfos;
        for (uint32_t Index = 0; Index < Payload->WaitSemaphores.size(); ++Index)
        {
            VkSemaphoreSubmitInfo WaitSemaphoreSubmitInfo = {};
            WaitSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            WaitSemaphoreSubmitInfo.semaphore = Payload->WaitSemaphores[Index];
            WaitSemaphoreSubmitInfo.stageMask = Payload->WaitStageMasks[Index];
            WaitSemaphoreSubmitInfos.push_back(WaitSemaphoreSubmitInfo);
        }

        std::vector<VkSemaphoreSubmitInfo> SignalSemaphoreSubmitInfos;
        for (uint32_t Index = 0; Index < Payload->SignalSemaphores.size(); ++Index)
        {
            VkSemaphoreSubmitInfo ReadySemaphoreSubmitInfo = {};
            ReadySemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            ReadySemaphoreSubmitInfo.semaphore = Payload->SignalSemaphores[Index];
            ReadySemaphoreSubmitInfo.stageMask = Payload->SignalStageMasks[Index];
            SignalSemaphoreSubmitInfos.push_back(ReadySemaphoreSubmitInfo);
        }

        std::vector<VkCommandBufferSubmitInfo> CommandBufferSubmitInfos;
        for (FVulkanCommandBuffer *CommandBuffer : Payload->CommandBuffers)
        {
            VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
            CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            CommandBufferSubmitInfo.commandBuffer = CommandBuffer->GetHandle();
            CommandBufferSubmitInfos.push_back(CommandBufferSubmitInfo);
            CommandBuffer->Submit();
        }

        uint64_t TimelineValue = NextTimelineSemaphoreValue++;
        Payload->TimelineSemaphoreValue = TimelineValue;

        VkSemaphoreSubmitInfo TimelineSemaphoreSubmitInfo = {};
        TimelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        TimelineSemaphoreSubmitInfo.semaphore = TimelineSemaphore;
        TimelineSemaphoreSubmitInfo.value = TimelineValue;
        TimelineSemaphoreSubmitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR;
        SignalSemaphoreSubmitInfos.push_back(TimelineSemaphoreSubmitInfo);

        VkSubmitInfo2 SubmitInfo = {};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        SubmitInfo.waitSemaphoreInfoCount = WaitSemaphoreSubmitInfos.size();
        SubmitInfo.pWaitSemaphoreInfos = WaitSemaphoreSubmitInfos.data();
        SubmitInfo.signalSemaphoreInfoCount = SignalSemaphoreSubmitInfos.size();
        SubmitInfo.pSignalSemaphoreInfos = SignalSemaphoreSubmitInfos.data();
        SubmitInfo.commandBufferInfoCount = CommandBufferSubmitInfos.size();
        SubmitInfo.pCommandBufferInfos = CommandBufferSubmitInfos.data();

        VkResult Result = VulkanRHI::vkQueueSubmit2(Handle, 1, &SubmitInfo, nullptr);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to submit command buffer to graphics queue.");

        SubmissionQueue.push(Payload);
    }
}
void FVulkanQueue::ImmediateSubmit(std::function<void(FRHICommandList &CmdList)> InCallback)
{
    FVulkanCommandListContext CmdListContext(Device);
    FRHICommandList CmdList(CmdListContext);

    InCallback(CmdList);

    FRHICommandListExecutor::Execute(CmdList.GetContext(), CmdList.Flush());

    std::vector<FVulkanCommandBufferPayload*> Payloads;
    CmdListContext.Finalize(Payloads);

    for (FVulkanCommandBufferPayload* Payload : Payloads)
    {
        std::vector<VkCommandBufferSubmitInfo> CommandBufferSubmitInfos;
        for (FVulkanCommandBuffer* CommandBuffer : Payload->CommandBuffers)
        {
            VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
            CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
            CommandBufferSubmitInfo.commandBuffer = CommandBuffer->GetHandle();
            CommandBufferSubmitInfos.push_back(CommandBufferSubmitInfo);
            CommandBuffer->Submit();
        }

        VkFenceCreateInfo FenceCreateInfo = {};
        FenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        VkFence Fence;
        VulkanRHI::vkCreateFence(Device.GetVkDevice(), &FenceCreateInfo, nullptr, &Fence);

        VkSubmitInfo2 SubmitInfo = {};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        SubmitInfo.commandBufferInfoCount = CommandBufferSubmitInfos.size();
        SubmitInfo.pCommandBufferInfos = CommandBufferSubmitInfos.data();

        VkResult Result = VulkanRHI::vkQueueSubmit2(Handle, 1, &SubmitInfo, Fence);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to submit to queue.");

        VulkanRHI::vkWaitForFences(Device.GetVkDevice(), 1, &Fence, VK_TRUE, UINT64_MAX);
        VulkanRHI::vkDestroyFence(Device.GetVkDevice(), Fence, nullptr);
    }
}

void FVulkanQueue::Wait()
{
    if (SubmissionQueue.empty())
    {
        return;
    }

    FVulkanCommandBufferPayload* Payload = SubmissionQueue.front();
    SubmissionQueue.pop();

    VkSemaphoreWaitInfo SemaphoreWaitInfo = {};
    SemaphoreWaitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    SemaphoreWaitInfo.semaphoreCount = 1;
    SemaphoreWaitInfo.pSemaphores    = &TimelineSemaphore;
    SemaphoreWaitInfo.pValues        = &Payload->TimelineSemaphoreValue;
    VulkanRHI::vkWaitSemaphores(GetVulkanRHIMinimal()->RHIGetVkDevice(), &SemaphoreWaitInfo, UINT64_MAX);

    for (FVulkanCommandBuffer* CB : Payload->CommandBuffers)
    {
        CB->Finish();

        CommandBufferPool->Recycle(CB);
    }

    delete Payload;
}

FVulkanCommandBufferPool* FVulkanQueue::GetCommandBufferPool() const
{
    return CommandBufferPool;
}

void FVulkanQueue::Init()
{
    VulkanRHI::vkGetDeviceQueue(Device .GetVkDevice(), QueueFamilyIndex, 0, &Handle);

    VkSemaphoreTypeCreateInfo TimelineSemaphoreCreateInfo = {};
    TimelineSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    TimelineSemaphoreCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    TimelineSemaphoreCreateInfo.initialValue  = 0;

    VkSemaphoreCreateInfo SemaphoreCreateInfo = {};
    SemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    SemaphoreCreateInfo.pNext = &TimelineSemaphoreCreateInfo;
    VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreCreateInfo, nullptr, &TimelineSemaphore);

    CommandBufferPool = new FVulkanCommandBufferPool(Device, *this);
}

void FVulkanQueue::Shutdown()
{
    while (!SubmissionQueue.empty())
    {
        FVulkanCommandBufferPayload* Payload = SubmissionQueue.front();
        SubmissionQueue.pop();
        delete Payload;
    }

    delete CommandBufferPool;

    VulkanRHI::vkDestroySemaphore(Device.GetVkDevice(), TimelineSemaphore, nullptr);
}
