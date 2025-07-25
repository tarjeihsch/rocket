#include "VulkanRenderGraph.h"

#include "VulkanLoader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandList.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "Assert.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanResourceManager.h"
#include "VulkanDevice.h"

void FVulkanRGBuilder::AddPass(std::string Name, EVulkanQueueType Queue, std::span<const FRGResourceAccess> Reads, std::span<const FRGResourceAccess> Writes, std::function<void(FRHICommandList&)>&& Lambda)
{
    FRGPass Pass = {};
    Pass.Name = Name;
    Pass.Reads.assign(Reads.begin(), Reads.end());
    Pass.Queue = Queue;
    Pass.Writes.assign(Writes.begin(), Writes.end());
    Pass.RecordFn = std::move(Lambda);
    RenderGraph.Passes.push_back(Pass);
}

void FVulkanRGBuilder::Import(ResourceHandle p_Handle)
{
    RenderGraph.Resources.push_back(p_Handle);
}

FVulkanRenderGraph::FVulkanRenderGraph(FVulkanDevice& p_Device)
    : m_Device(p_Device), Builder(*this)
{
}

FVulkanRenderGraph::~FVulkanRenderGraph()
{
}

FVulkanRGBuilder &FVulkanRenderGraph::GetMutableBuilder()
{
    return Builder;
}

void FVulkanRenderGraph::Begin()
{
//    Ctx.AddWaitSemaphore(Viewport->GetImageAcquiredSemaphore());
}

void FVulkanRenderGraph::Reset()
{
//    Ctx.AddSignalSemaphore(Viewport->GetRenderFinishedSemaphore());

    Passes.clear();
    Resources.clear();
    CompiledPayloads.clear();
}

void FVulkanRenderGraph::Compile()
{
    CompiledPayloads.clear();
    CompiledPayloads.reserve(Passes.size());

    //------------------------------------------------------------
    // State we track while walking the passes
    //------------------------------------------------------------
    struct FImgState
    {
        VkImageLayout     CurrentLayout       = VK_IMAGE_LAYOUT_UNDEFINED;
        EVulkanQueueType  CurrentQueue        = EVulkanQueueType::Graphics;
        int32_t           LastPayloadIndex    = -1;   // where it was last written/read
        bool              FirstUse            = true;
    };
    std::unordered_map<uint32_t, FImgState> ImageStates;   // key = resource index

    //------------------------------------------------------------
    // For now: one payload == one pass
    //------------------------------------------------------------
    for (uint32_t PassIdx = 0; PassIdx < Passes.size(); ++PassIdx)
    {
        const FRGPass& P = Passes[PassIdx];

        FRGCompiledPayload Payload;
        Payload.PassIndices.push_back(PassIdx);
        Payload.Queue        = P.Queue;

        //------------------------------------------------------------------
        // Helper lambda that processes a single Read or Write declaration
        //------------------------------------------------------------------
        auto HandleAccess = [&](const FRGResourceAccess& A, bool bIsWrite)
        {
            if (A.Handle.Type != ResourceType::Image)
                return;   // buffer support can be added later

            auto& ImgState = ImageStates[A.Handle.Index];
            const VkImage imgHandle = m_Device.GetResourceManager()->GetImage(Resources[A.Handle.Index]);

            if (ImgState.FirstUse && ImgState.CurrentLayout == VK_IMAGE_LAYOUT_UNDEFINED)
            {
                //ImgState.CurrentLayout = Resources[A.Handle.Index].InitialLayout;
                ImgState.CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            }


            if (ImgState.FirstUse)
            {
                if (ImgState.CurrentLayout != A.Layout)
                {
                    FRGBarrier B;
                    B.Image      = imgHandle;
                    B.OldLayout  = ImgState.CurrentLayout;   // UNDEFINED
                    B.NewLayout  = A.Layout;
                    B.SrcStage   = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;      // nothing to wait for
                    B.DstStage   = A.StageMask;                            // what this pass needs
                    Payload.BeginBarriers.push_back(B);
                }

                ImgState.CurrentLayout    = A.Layout;
                ImgState.CurrentQueue     = P.Queue;
                ImgState.LastPayloadIndex = static_cast<int32_t>(CompiledPayloads.size());
                ImgState.FirstUse         = false;
                return;                     // DONE – no “diff” logic needed
            }

            //-----------------------------------------------------------
            // If this is NOT the first use, check whether we need:
            //  * a layout transition             (layout differs)
            //  * a queue ownership transfer      (queue differs)
            //-----------------------------------------------------------
            if (!ImgState.FirstUse)
            {
                const bool LayoutDiff = (ImgState.CurrentLayout != A.Layout);
                const bool QueueDiff  = (ImgState.CurrentQueue  != P.Queue);

                if (LayoutDiff || QueueDiff)
                {
                    FRGBarrier B;
                    B.Image      = imgHandle;
                    B.OldLayout  = ImgState.CurrentLayout;
                    B.NewLayout  = A.Layout;
                    B.SrcStage   = ImgState.CurrentQueue == P.Queue
                                   ? A.StageMask        // same queue
                                   : VK_PIPELINE_STAGE_ALL_COMMANDS_BIT; // ownership xfer
                    B.DstStage   = A.StageMask;

                    //------------------------------------------------------------------
                    // Decide where to emit the barrier:
                    //   * same payload  ->   begin-barrier inside this payload
                    //   * different payloads -> end-barrier of producer payload
                    //------------------------------------------------------------------
                    if (ImgState.LastPayloadIndex == static_cast<int32_t>(CompiledPayloads.size()))
                    {
                        Payload.BeginBarriers.push_back(B);
                    }
                    else
                    {
                        CompiledPayloads[ImgState.LastPayloadIndex].EndBarriers.push_back(B);

                        // Queue ownership transfer ⇒ semaphore
                        if (QueueDiff)
                        {
                            // NOTE: You probably have a semaphore pool already.
                            //       For now, pretend we fetch a binary semaphore:
                            // Create a new binary semaphore
                            VkSemaphoreCreateInfo CI{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
                            VkSemaphore Sem;
                            VulkanRHI::vkCreateSemaphore(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CI, nullptr, &Sem);

//                            VkSemaphore xferSem = AcquireTempSemaphore();
                            VkSemaphore xferSem = Sem;

                            CompiledPayloads[ImgState.LastPayloadIndex].Signal.push_back(xferSem);
                            Payload.Wait.push_back(xferSem);
                        }
                    }
                }
            }

            //-----------------------------------------------------------
            // Update state for next pass that touches this image
            //-----------------------------------------------------------
            ImgState.CurrentLayout    = A.Layout;
            ImgState.CurrentQueue     = P.Queue;
            ImgState.LastPayloadIndex = static_cast<int32_t>(CompiledPayloads.size());
            ImgState.FirstUse         = false;
        };

        for (const FRGResourceAccess& R : P.Reads)  HandleAccess(R, /*isWrite=*/false);
        for (const FRGResourceAccess& W : P.Writes) HandleAccess(W, /*isWrite=*/true);

        CompiledPayloads.push_back(std::move(Payload));
    }
}

void FVulkanRenderGraph::Execute(FVulkanCommandBufferContext& FrameCtx)
{
    for (FRGCompiledPayload Plan : CompiledPayloads)
    {
        if (!Plan.Wait.empty())
        {
            FrameCtx.AddWaitSemaphore(std::span<VkSemaphore>(Plan.Wait.data(), Plan.Wait.size()));
        }

        FVulkanCommandBuffer* CB = FrameCtx.GetCurrentCommandBuffer();
        FVulkanCommandListContext CmdCtx(CB->GetHandle());
        FRHICommandList CmdList(CmdCtx);

        for (const FRGBarrier& B : Plan.BeginBarriers)
        {
            CmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(B.Image, B.OldLayout, B.NewLayout);
        }

        for (uint32_t Index = 0; Index < Plan.PassIndices.size(); Index++)
        {
            Passes[Plan.PassIndices[Index]].RecordFn(CmdList);
        }

        for (const FRGBarrier& B : Plan.EndBarriers)
        {
            CmdList.Enqueue<FVulkanCommandSetMemoryBarrier>(B.Image, B.OldLayout, B.NewLayout);
        }

        if (!Plan.Signal.empty())
        {
            FrameCtx.AddSignalSemaphore(std::span<VkSemaphore>(Plan.Signal.data(), Plan.Signal.size()));
        }

        FRHICommandListExecutor::Execute(CmdCtx, CmdList.Flush());
    }

    // todo: calculate vma statistics at the end of the frame.
}
