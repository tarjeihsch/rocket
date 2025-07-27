#include "VulkanFrameGraph.h"

#include "VulkanLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandList.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "Assert.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanResourceManager.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"

void VulkanFrameGraph::AddPass(std::string Name, FrameGraphSubmissionQueue Queue, std::span<const FrameGraphResourceAccess> Reads, std::span<const FrameGraphResourceAccess> Writes, std::function<void(FRHICommandList&)>&& Lambda)
{
    FrameGraphPass Pass = {};
    Pass.Name = Name;
    Pass.Inputs.assign(Reads.begin(), Reads.end());
    Pass.Queue = Queue;
    Pass.Outputs.assign(Writes.begin(), Writes.end());
    Pass.Callback = std::move(Lambda);
    Passes.push_back(Pass);
}

VulkanFrameGraph::VulkanFrameGraph(FVulkanDevice& p_Device)
    : m_Device(p_Device)
{
}

VulkanFrameGraph::~VulkanFrameGraph()
{
}

void VulkanFrameGraph::HandleResourceAccess(const FrameGraphPass& Pass, FrameGraphPayload& Payload, const FrameGraphResourceAccess& ResourceAccess, bool IsWrite)
{
    if (ResourceAccess.Handle.Type == ResourceType::Image)
    {
        auto& State = m_Compiler.ImageStates[ResourceAccess.Handle.Index];
        VkImage Image = m_Device.GetResourceManager()->GetImage(ResourceAccess.Handle);

        if (State.FirstUse)
        {
            if (State.CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                State.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }

            if (State.CurrentLayout != ResourceAccess.Layout)
            {
                FrameGraphBarrier Barrier;
                Barrier.Image      = Image;
                Barrier.OldLayout  = State.CurrentLayout;
                Barrier.NewLayout  = ResourceAccess.Layout;
                Barrier.SrcStage   = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                Barrier.DstStage   = ResourceAccess.StageMask;
                Payload.BeginBarriers.push_back(Barrier);
            }
        }
        else
        {
            const bool LayoutDiff = (State.CurrentLayout != ResourceAccess.Layout);
            const bool QueueDiff  = (State.CurrentQueue  != Pass.Queue);

            if (LayoutDiff || QueueDiff)
            {
                FrameGraphBarrier Barrier;
                Barrier.Image      = Image;
                Barrier.OldLayout  = State.CurrentLayout;
                Barrier.NewLayout  = ResourceAccess.Layout;
                Barrier.SrcStage   = State.CurrentQueue == Pass.Queue
                               ? ResourceAccess.StageMask        // same queue
                               : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // ownership xfer
                Barrier.DstStage   = ResourceAccess.StageMask;

                //------------------------------------------------------------------
                // Decide where to emit the barrier:
                //   * same payload  ->   begin-barrier inside this payload
                //   * different payloads -> end-barrier of producer payload
                //------------------------------------------------------------------
                if (State.LastPayloadIndex == static_cast<int32_t>(Payloads.size()))
                {
                    Payload.BeginBarriers.push_back(Barrier);
                }
                else
                {
                    Payloads[State.LastPayloadIndex].EndBarriers.push_back(Barrier);

                    // Queue ownership transfer ⇒ semaphore
                    if (QueueDiff)
                    {
                        // NOTE: You probably have a semaphore pool already.
                        //       For now, we just create a new binary semaphore.
                        VkSemaphoreCreateInfo CI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
                        VkSemaphore Sem;
                        VulkanRHI::vkCreateSemaphore(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CI, nullptr, &Sem);

                        VkSemaphore xferSem = Sem;

                        Payloads[State.LastPayloadIndex].Signal.push_back(xferSem);
                        Payload.Wait.push_back(xferSem);
                    }
                }
            }
        }

        State.CurrentLayout    = ResourceAccess.Layout;
        State.CurrentQueue     = Pass.Queue;
        State.LastPayloadIndex = static_cast<int32_t>(Payloads.size());
        State.FirstUse         = false;
    }
}

void VulkanFrameGraph::Compile()
{
    Payloads.reserve(Passes.size());

    for (uint32_t PassIndex = 0; PassIndex < Passes.size(); ++PassIndex)
    {
        const FrameGraphPass& Pass = Passes[PassIndex];

        FrameGraphPayload Payload;
        Payload.PassIndices.push_back(PassIndex);
        Payload.Queue = Pass.Queue;

        for (const FrameGraphResourceAccess& Input : Pass.Inputs)
        {
            HandleResourceAccess(Pass, Payload, Input, false);
        }

        for (const FrameGraphResourceAccess& Output : Pass.Outputs)
        {
            HandleResourceAccess(Pass, Payload, Output, true);
        }

        Payloads.push_back(std::move(Payload));
    }
}

void VulkanFrameGraph::Execute(VkSemaphore ImageAcquiredSemaphore, VkSemaphore RenderFinishedSemaphore)
{
    FVulkanCommandBufferPool* GraphicsCommandBufferPool = m_Device.GetGraphicsQueue()->GetCommandBufferPool();
    FVulkanCommandBufferPool* ComputesCommandBufferPool = m_Device.GetComputeQueue()->GetCommandBufferPool();
    FVulkanCommandBufferPool* TransferCommandBufferPool = m_Device.GetTransferQueue()->GetCommandBufferPool();

    FVulkanCommandBufferContext GfxFrameCtx(*GraphicsCommandBufferPool);
    FVulkanCommandBufferContext ComputeFrameCtx(*ComputesCommandBufferPool);
    FVulkanCommandBufferContext TransferFrameCtx(*TransferCommandBufferPool);

    FVulkanCommandBufferContext* CurrentCtx = nullptr;

    GfxFrameCtx.AddWaitSemaphore(ImageAcquiredSemaphore);

    for (FrameGraphPayload& Plan : Payloads)
    {
        switch (Plan.Queue)
        {
        case FrameGraphSubmissionQueue::Graphics: CurrentCtx = &GfxFrameCtx; break;
        case FrameGraphSubmissionQueue::Compute: CurrentCtx = &ComputeFrameCtx; break;
        case FrameGraphSubmissionQueue::Transfer: CurrentCtx = &TransferFrameCtx; break;
        }

        if (!Plan.Wait.empty())
        {
            CurrentCtx->AddWaitSemaphore(std::span<VkSemaphore>(Plan.Wait.data(), Plan.Wait.size()));
        }

        FVulkanCommandBuffer* CB = CurrentCtx->GetCurrentCommandBuffer();
        FVulkanCommandListContext CmdCtx(CB->GetHandle());
        FRHICommandList CmdList(CmdCtx);

        for (const FrameGraphBarrier& B : Plan.BeginBarriers)
        {
            CmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(B.Image, B.OldLayout, B.NewLayout);
        }

        for (uint32_t Index = 0; Index < Plan.PassIndices.size(); Index++)
        {
            Passes[Plan.PassIndices[Index]].Callback(CmdList);
        }

        for (const FrameGraphBarrier& B : Plan.EndBarriers)
        {
            CmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(B.Image, B.OldLayout, B.NewLayout);
        }

        if (!Plan.Signal.empty())
        {
            CurrentCtx->AddSignalSemaphore(std::span<VkSemaphore>(Plan.Signal.data(), Plan.Signal.size()));
        }

        FRHICommandListExecutor::Execute(CmdCtx, CmdList.Flush());
    }

    GfxFrameCtx.AddSignalSemaphore(RenderFinishedSemaphore);

    m_Device.GetGraphicsQueue()->Submit(GfxFrameCtx);
    m_Device.GetComputeQueue()->Submit(ComputeFrameCtx);
    m_Device.GetTransferQueue()->Submit(TransferFrameCtx);
}
