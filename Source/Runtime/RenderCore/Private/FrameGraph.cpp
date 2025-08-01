#include "FrameGraph.h"

#include "Common/RHI.h"
#include "Common/RHICommandList.h"
#include "Common/RHITexture.h"

#include <cassert>

FFGTextureRef::FFGTextureRef(ERHIResourceAccess InAccess, FRHITexture *InHandle)
    : FFGResourceRef(InAccess), Handle(InHandle)
{
    RTVHandle.reset(GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color), Handle));
    SRVHandle.reset(GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color), Handle));
    UAVHandle.reset(GetRHI()->RHICreateTextureView(FRHITextureViewDesc::Create2D(ERHITextureFormat::B8G8R8A8_SRGB, ERHITextureAspect::Color), Handle));
}

void FFrameGraphPass::AddColorAttachment(FRHITexture* Resource)
{
    TextureWrites.push_back({ ERHIResourceAccess::ColorAttachmentWrite, Resource });
}

void FFrameGraphPass::AddInputAttachment(FRHITexture* Resource)
{
    TextureReads.push_back({ ERHIResourceAccess::ShaderSampledRead, Resource });
}

void FFrameGraphPass::AddSampledTexture(FRHITexture* Resource)
{
    TextureReads.push_back({ ERHIResourceAccess::ShaderSampledRead, Resource });
}

void FFrameGraphPass::AddDepthAttachment(FRHITexture* Resource)
{
    TextureWrites.push_back({ ERHIResourceAccess::DepthStencilAttachmentWrite, Resource });
}

void FFrameGraphPass::AddStorageImage(FRHITexture* Resource)
{
    TextureWrites.push_back({ ERHIResourceAccess::ShaderStorageWrite, Resource });
}

void FFrameGraphPass::AddTransferSrc(FRHITexture* Resource)
{
    TextureReads.push_back({ERHIResourceAccess::TransferRead, Resource});
}


void FFrameGraphPass::AddTransferDst(FRHITexture* Resource)
{
    TextureWrites.push_back({ERHIResourceAccess::TransferWrite, Resource});
}

void FFrameGraphPass::AddPresentSrc(FRHITexture* Resource)
{
    TextureWrites.push_back({ERHIResourceAccess::Presentation, Resource});
}

FFrameGraphBuilder::FFrameGraphBuilder()
{
}

FFrameGraphBuilder::~FFrameGraphBuilder()
{
}

FFrameGraphPass& FFrameGraphBuilder::AddPass(const char *DebugName, EFGPassFlags Flags, std::function<void(FRHICommandList &CmdList)> &&Callback)
{
    FFrameGraphPass Pass = { DebugName, Flags, std::move(Callback), };
    Passes.push_back(std::move(Pass));
    return Passes.back();
}

void FFrameGraphBuilder::Execute()
{
    for (FFrameGraphPass& Pass : Passes)
    {
        ERHITextureLayout NewLayout;
        ERHIResourceAccess NewAccess;
        ERHIPipelineStage NewStage;

        IRHICommandListContext* CmdListContext = GetRHI()->GetCommandListContext();
        FRHICommandList CmdList(*CmdListContext);

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

        std::vector<FRHITextureView*> TextureViews;

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
            case ERHIResourceAccess::Presentation:
                NewLayout = ERHITextureLayout::Present;
                NewAccess = ERHIResourceAccess::None;
                NewStage = ERHIPipelineStage::BottomOfPipe;
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

            if (NewAccess == ERHIResourceAccess::ColorAttachmentWrite || NewAccess == ERHIResourceAccess::DepthStencilAttachmentWrite)
            {
                TextureViews.push_back(Resource.UAVHandle.get());
            }
        }

        if ((Pass.Flags & EFGPassFlags::Dynamic) == EFGPassFlags::Dynamic)
        {
            GetRHI()->RHICmdBeginRendering(&CmdList, TextureViews);
        }

        if (Pass.Callback)
        {
            Pass.Callback(CmdList);
        }

        if ((Pass.Flags & EFGPassFlags::Dynamic) == EFGPassFlags::Dynamic)
        {
            GetRHI()->RHICmdEndRendering(&CmdList);
        }

        FRHICommandListExecutor::Execute(CmdList.GetContext(), CmdList.Flush());
    }
}
