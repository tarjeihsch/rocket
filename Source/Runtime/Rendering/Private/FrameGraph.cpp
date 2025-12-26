#include "FrameGraph.h"

#include "Common/RHI.h"
#include "Common/RHICommandList.h"
#include "Common/RHITexture.h"

#include <cassert>

#include "Common/RHISemaphore.h"
#include "Common/RHIViewport.h"

static void ApplyBarrier(FRHICommandList& CmdList, FRHITexture* Texture, ERHITextureLayout NewLayout, ERHIResourceAccess NewAccess, ERHIPipelineStage NewStage)
{
    if (Texture->CurrentLayout != NewLayout || Texture->CurrentAccess != NewAccess || Texture->CurrentPipelineStage != NewStage)
    {
        GetRHI()->RHICmdPipelineBarrier(&CmdList, Texture, Texture->CurrentLayout, NewLayout, Texture->CurrentAccess, NewAccess, Texture->CurrentPipelineStage, NewStage);

        Texture->CurrentLayout = NewLayout;
        Texture->CurrentAccess = NewAccess;
        Texture->CurrentPipelineStage = NewStage;
    }
}

FFGTextureRef::FFGTextureRef(ERHIResourceAccess InAccess, FRHITexture* InHandle)
    : FFGResourceRef(InAccess), Handle(InHandle)
{
    // TODO: switch
    if (Handle->RHIGetTextureDesc().Format == ERHITextureFormat::B8G8R8A8_SRGB)
    {
        RTVHandle = GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color, ERHIResourceLifetime::Transient), Handle);
    }
    else if (Handle->RHIGetTextureDesc().Format == ERHITextureFormat::D32_SFLOAT)
    {
        DSVHandle = GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::D32_SFLOAT, ERHITextureAspect::Depth, ERHIResourceLifetime::Transient), Handle);
    }
    else if (Handle->RHIGetTextureDesc().Format == ERHITextureFormat::D32_SFLOAT_S8_UINT)
    {
        DSVHandle = GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::D32_SFLOAT_S8_UINT, ERHITextureAspect::DepthStencil, ERHIResourceLifetime::Transient), Handle);
    }
}

FFGTextureRef::~FFGTextureRef()
{
    // TODO: Vector resizing will call DTOR on texture view objects.
    // Will cause duplication of objects.
    delete RTVHandle;
    delete SRVHandle;
    delete UAVHandle;
    delete DSVHandle;
}

void FFrameGraphPass::AddColorAttachment(FRHITexture* Resource)
{
    TextureWrites.emplace_back( ERHIResourceAccess::ColorAttachmentWrite, Resource);
}

void FFrameGraphPass::AddInputAttachment(FRHITexture* Resource)
{
    TextureReads.emplace_back(ERHIResourceAccess::ShaderSampledRead, Resource);
}

void FFrameGraphPass::AddSampledTexture(FRHITexture* Resource)
{
    TextureReads.emplace_back(ERHIResourceAccess::ShaderSampledRead, Resource);
}

void FFrameGraphPass::AddDepthAttachment(FRHITexture* Resource)
{
    TextureWrites.emplace_back(ERHIResourceAccess::DepthStencilAttachmentWrite, Resource);
}

void FFrameGraphPass::AddStorageImage(FRHITexture* Resource)
{
    TextureWrites.emplace_back(ERHIResourceAccess::ShaderStorageWrite, Resource);
}

void FFrameGraphPass::AddTransferSrc(FRHITexture* Resource)
{
    TextureReads.emplace_back(ERHIResourceAccess::TransferRead, Resource);
}

void FFrameGraphPass::AddTransferDst(FRHITexture* Resource)
{
    TextureWrites.emplace_back(ERHIResourceAccess::TransferWrite, Resource);
}

FFrameGraphPass& FFrameGraphBuilder::AddPass(const char *DebugName, EFGPassFlags Flags, std::function<void(FRHICommandList &CmdList)> &&Callback)
{
    FFrameGraphPass Pass;
    Pass.DebugName = DebugName;
    Pass.Flags = Flags;
    Pass.Callback = std::move(Callback);
    Passes.emplace_back(Pass);
    return Passes.back();
}

void FFrameGraphBuilder::Execute()
{
    IRHICommandListContext* CmdListContext = GetRHI()->GetCommandListContext();
    FRHICommandList CmdList(*CmdListContext);

    for (FFrameGraphPass& Pass : Passes)
    {
        ERHITextureLayout NewLayout;
        ERHIResourceAccess NewAccess;
        ERHIPipelineStage NewStage;

        for (const FFGTextureRef& Resource : Pass.TextureReads)
        {
            switch (Resource.TargetAccess)
            {
            case ERHIResourceAccess::ShaderSampledRead:
                NewLayout = ERHITextureLayout::ShaderReadOnly;
                NewAccess = ERHIResourceAccess::ShaderSampledRead;
                NewStage = ERHIPipelineStage::FragmentShader;
                break;
            case ERHIResourceAccess::InputAttachmentRead:
                NewLayout = ERHITextureLayout::ShaderReadOnly;
                NewAccess = ERHIResourceAccess::InputAttachmentRead;
                NewStage = ERHIPipelineStage::FragmentShader;
                break;
            case ERHIResourceAccess::TransferRead:
                NewLayout = ERHITextureLayout::TransferSrc;
                NewAccess = ERHIResourceAccess::TransferRead;
                NewStage = ERHIPipelineStage::Transfer;
                break;
            default:
                assert(false && "Unhandled EFGAccessType in ReadUsage");
                break;
            }

            if (Resource.Handle->CurrentLayout != NewLayout || Resource.Handle->CurrentAccess != NewAccess || Resource.Handle->CurrentPipelineStage != NewStage)
            {
                GetRHI()->RHICmdPipelineBarrier(&CmdList, Resource.Handle, Resource.Handle->CurrentLayout, NewLayout, Resource.Handle->CurrentAccess, NewAccess, Resource.Handle->CurrentPipelineStage, NewStage);

                Resource.Handle->CurrentLayout = NewLayout;
                Resource.Handle->CurrentAccess = NewAccess;
                Resource.Handle->CurrentPipelineStage = NewStage;
            }
        }

        std::vector<FRHITextureView*> ColorAttachments;
        FRHITextureView* DepthAttachmentView = nullptr;

        for (const FFGTextureRef& Resource : Pass.TextureWrites)
        {
            switch (Resource.TargetAccess)
            {
            case ERHIResourceAccess::ColorAttachmentWrite:
                NewLayout = ERHITextureLayout::ColorAttachment;
                NewAccess = ERHIResourceAccess::ColorAttachmentWrite;
                NewStage = ERHIPipelineStage::ColorAttachment;
                break;
            case ERHIResourceAccess::DepthStencilAttachmentWrite:
                NewLayout = ERHITextureLayout::DepthStencil;
                NewAccess = ERHIResourceAccess::DepthStencilAttachmentWrite;
                NewStage = ERHIPipelineStage::DepthStencil;
                break;
            case ERHIResourceAccess::ShaderStorageWrite:
                NewLayout = ERHITextureLayout::General;
                NewAccess = ERHIResourceAccess::ShaderStorageWrite;
                NewStage = ERHIPipelineStage::ComputeShader;
                break;
            case ERHIResourceAccess::TransferWrite:
                NewLayout = ERHITextureLayout::TransferDst;
                NewAccess = ERHIResourceAccess::TransferWrite;
                NewStage = ERHIPipelineStage::Transfer;
                break;
            default:
                assert(false && "Unhandled EFGAccessType in ReadUsage");
                break;
            }

            if (Resource.Handle->CurrentLayout != NewLayout || Resource.Handle->CurrentAccess != NewAccess || Resource.Handle->CurrentPipelineStage != NewStage)
            {
                GetRHI()->RHICmdPipelineBarrier(&CmdList, Resource.Handle, Resource.Handle->CurrentLayout, NewLayout, Resource.Handle->CurrentAccess, NewAccess, Resource.Handle->CurrentPipelineStage, NewStage);

                Resource.Handle->CurrentLayout = NewLayout;
                Resource.Handle->CurrentAccess = NewAccess;
                Resource.Handle->CurrentPipelineStage = NewStage;
            }

            if (NewAccess == ERHIResourceAccess::ColorAttachmentWrite)
            {
                ColorAttachments.emplace_back(Resource.RTVHandle);
            }

            if (NewAccess == ERHIResourceAccess::DepthStencilAttachmentWrite)
            {
                DepthAttachmentView = Resource.DSVHandle;
            }
        }

        if ((Pass.Flags & EFGPassFlags::Dynamic) == EFGPassFlags::Dynamic)
        {
            GetRHI()->RHICmdBeginRendering(&CmdList, ColorAttachments, DepthAttachmentView, (Pass.Flags & EFGPassFlags::ClearColor) == EFGPassFlags::ClearColor);
        }

        if (Pass.Callback)
        {
            Pass.Callback(CmdList);
        }

        if ((Pass.Flags & EFGPassFlags::Dynamic) == EFGPassFlags::Dynamic)
        {
            GetRHI()->RHICmdEndRendering(&CmdList);
        }

        /*
         * GetRHI()->CreateRenderPass([](){
         *      Blit(Composition, BackBuffer);
         * });
         *
         */

        //for (RRHIViewport* Viewport : Pass.PresentationTargets)
        //{
        //    Viewport->Acquire();

        //    FRHITexture* Texture = Viewport->GetBackBufferTexture();
        //    ApplyBarrier(CmdList, Texture, ERHITextureLayout::Present, ERHIResourceAccess::None, ERHIPipelineStage::BottomOfPipe);

        //    // Wait, Signal and Present are deferred until Flush():
        //    //  Wait semaphores are injected into the first payload, while signal semaphores are injected into the last.
        //    CmdListContext->RHIAddPendingSubmitWait(Viewport->GetImageAvailableSemaphore().get(), ERHIQueueSyncStage::ColorAttachmentOutput);
        //    CmdListContext->RHIAddPendingSubmitSignal(Viewport->GetRenderFinishedSemaphore().get(), ERHIQueueSyncStage::AllCommands);
        //    CmdListContext->RHIAddPendingSubmitPresent(Viewport->GetBackBufferTexture().get());
        //}
    }

    // Fetch viewports related to this frame

    FRHICommandListExecutor::Execute(CmdList.GetContext(), CmdList.Flush());
}
