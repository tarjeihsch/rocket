#include "VulkanFrameGraph.h"

#include "Assert.h"
#include "VulkanCommandBuffer.h"
#include "VulkanCommandBufferContext.h"
#include "VulkanCommandList.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanTexture.h"

#include <algorithm>
#include <cassert>

void SetImageBarrier(
    VkCommandBuffer CommandBuffer,
    VkImage Image,
    VkImageLayout OldLayout,
    VkImageLayout NewLayout,
    VkAccessFlags SrcAccess,
    VkAccessFlags DstAccess,
    VkPipelineStageFlags SrcStage,
    VkPipelineStageFlags DstStage,
    uint32_t SrcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    uint32_t DstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED)
{
    VkImageMemoryBarrier2 imageBarrier = {};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    imageBarrier.srcStageMask = SrcStage;
    imageBarrier.srcAccessMask = SrcAccess;
    imageBarrier.dstStageMask = DstStage;
    imageBarrier.dstAccessMask = DstAccess;
    imageBarrier.oldLayout = OldLayout;
    imageBarrier.newLayout = NewLayout;
    imageBarrier.srcQueueFamilyIndex = SrcQueueFamilyIndex;
    imageBarrier.dstQueueFamilyIndex = DstQueueFamilyIndex;
    imageBarrier.image = Image;

    imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageBarrier.subresourceRange.baseMipLevel = 0;
    imageBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    imageBarrier.subresourceRange.baseArrayLayer = 0;
    imageBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &imageBarrier;

    VulkanRHI::vkCmdPipelineBarrier2(CommandBuffer, &dependencyInfo);
}

FFGTextureRef::FFGTextureRef()
    : Handle(nullptr), CurrentImageLayout(VK_IMAGE_LAYOUT_UNDEFINED), CurrentAccessFlags(VK_ACCESS_NONE), CurrentPipelineStageFlags(VK_PIPELINE_STAGE_NONE)
{
}

FFGTextureRef FFrameGraph::CreateTexture(FRHITextureDesc InTextureDesc, FRHITextureViewDesc InTextureViewDesc)
{
    FVulkanTexture* Texture = new FVulkanTexture(m_Device, InTextureDesc);

    VkMemoryRequirements MemoryRequirements;
    VulkanRHI::vkGetImageMemoryRequirements(GetVulkanRHIMinimal()->RHIGetVkDevice(), Texture->Image, &MemoryRequirements);

    VmaVirtualAllocationCreateInfo AllocInfo = {};
    AllocInfo.size = MemoryRequirements.size;
    AllocInfo.alignment = MemoryRequirements.alignment;

    // We can safely throw away the allocation and offset info given by vmaVirtualAllocate, as we are resetting the entire allocator each frame.
    VmaVirtualAllocation Allocation;
    VkDeviceSize Offset;

    VkResult Result = vmaVirtualAllocate(m_Device.GetTextureScratchAllocator()->VirtualMemory, &AllocInfo, &Allocation, &Offset);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate transient image memory.");
    VulkanRHI::vkBindImageMemory(GetVulkanRHIMinimal()->RHIGetVkDevice(), Texture->Image, m_Device.GetTextureScratchAllocator()->PhysicalMemory, Offset);

    FVulkanTextureView* TextureView = new FVulkanTextureView(m_Device, InTextureViewDesc, Texture->Image);

    FFGTextureRef TextureRef;
    TextureRef.Handle = Texture;;
    TextureRef.View = TextureView;
    TextureRef.bTransient = true;
    TextureRef.ResourceType = EFGResourceType::Texture;
    return TextureRef;
}

FFGTextureRef FFrameGraph::ImportTexture(FVulkanTexture *InTexture)
{
    FFGTextureRef TextureRef;
    TextureRef.Handle = InTexture;
    TextureRef.bTransient = false;
    TextureRef.ResourceType = EFGResourceType::Texture;
    return TextureRef;
}

void FFGPass::AddColorAttachment(FFGResourceRef* Resource)
{
    Writes.push_back({ Resource, EFGAccessType::ColorAttachmentWrite });

    Flags = Flags | EFGPassFlags::Dynamic;
}

void FFGPass::AddInputAttachment(FFGResourceRef *Resource)
{
    Reads.push_back({ Resource, EFGAccessType::SampledRead });
}

void FFGPass::AddSampledTexture(FFGResourceRef *Resource)
{
    Reads.push_back({ Resource, EFGAccessType::SampledRead });
}

void FFGPass::AddDepthAttachment(FFGResourceRef *Resource)
{
    Writes.push_back({ Resource, EFGAccessType::DepthStencilWrite });
}

void FFGPass::AddStorageImage(FFGResourceRef *Resource)
{
    Writes.push_back({ Resource, EFGAccessType::StorageWrite });
}

void FFGPass::AddTransferDst(FFGResourceRef *Resource)
{
    Writes.push_back({ Resource, EFGAccessType::TransferWrite });
}

void FFGPass::AddTransferSrc(FFGResourceRef *Resource)
{
    Reads.push_back({ Resource, EFGAccessType::TransferRead });
}

void FFGPass::AddPresentSrc(FFGResourceRef *Resource)
{
    Writes.push_back({Resource, EFGAccessType::Present });
}

FFrameGraph::FFrameGraph(FVulkanDevice &p_Device)
    : m_Device(p_Device)
{
}

FFrameGraph::~FFrameGraph()
{
}

FFGPass &FFrameGraph::AddPass(const char *DebugName, std::function<void(FRHICommandList &CmdList)> &&Callback)
{
    FFGPass Pass = {DebugName};
    Pass.Callback = Callback;
    Passes.push_back(Pass);
    return Passes.back();
}

void FFrameGraph::Analyze()
{

}

void FFrameGraph::Execute()
{
    FVulkanCommandBufferContext GraphicsCommandBufferCtx(*m_Device.GetGraphicsQueue()->GetCommandBufferPool());

    for (FFGPass& Pass : Passes)
    {
        VkImageLayout NewLayout;
        VkAccessFlags NewAccess;
        VkPipelineStageFlags NewStage;

        FVulkanCommandBuffer* CommandBuffer = GraphicsCommandBufferCtx.GetCurrentCommandBuffer();
        FVulkanCommandListContext CommandListCtx(CommandBuffer->GetHandle());
        FRHICommandList CommandList(CommandListCtx);

        for (FFGResourceUsage& ReadUsage : Pass.Reads)
        {
            if (ReadUsage.Resource->IsTexture())
            {
                switch (ReadUsage.AccessType)
                {
                case EFGAccessType::SampledRead:
                    NewLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    NewAccess = VK_ACCESS_SHADER_READ_BIT;
                    NewStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    break;
                case EFGAccessType::InputAttachmentRead:
                    NewLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    NewAccess = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                    NewStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                    break;
                case EFGAccessType::TransferRead:
                    NewLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    NewAccess = VK_ACCESS_TRANSFER_READ_BIT;
                    NewStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    break;
                default:
                    assert(false && "Unhandled EFGAccessType in ReadUsage");
                    break;
                }

                FFGTextureRef* TextureRef = static_cast<FFGTextureRef*>(ReadUsage.Resource);

                if (TextureRef->CurrentImageLayout != NewLayout || TextureRef->CurrentAccessFlags != NewAccess || TextureRef->CurrentPipelineStageFlags != NewStage)
                {
                    SetImageBarrier(CommandBuffer->GetHandle(), TextureRef->Handle->Image, TextureRef->CurrentImageLayout, NewLayout, TextureRef->CurrentAccessFlags, NewAccess, TextureRef->CurrentPipelineStageFlags, NewStage);

                    TextureRef->CurrentImageLayout = NewLayout;
                    TextureRef->CurrentAccessFlags = NewAccess;
                    TextureRef->CurrentPipelineStageFlags = NewStage;
                }
            }
        }

        std::vector<VkImageView> ImageViews;

        for (FFGResourceUsage& WriteUsage : Pass.Writes)
        {
            if (WriteUsage.Resource->IsTexture())
            {
                switch (WriteUsage.AccessType)
                {
                case EFGAccessType::ColorAttachmentWrite:
                    NewLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    NewAccess = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    NewStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                    break;
                case EFGAccessType::DepthStencilWrite:
                    NewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    NewAccess = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                    NewStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                    break;
                case EFGAccessType::StorageWrite:
                    NewLayout = VK_IMAGE_LAYOUT_GENERAL;
                    NewAccess = VK_ACCESS_SHADER_WRITE_BIT;
                    NewStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                    break;
                case EFGAccessType::TransferWrite:
                    NewLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                    NewAccess = VK_ACCESS_TRANSFER_WRITE_BIT;
                    NewStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                    break;
                case EFGAccessType::Present:
                    NewLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                    NewAccess = 0;
                    NewStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                    break;
                default:
                    assert(false && "Unhandled EFGAccessType in ReadUsage");
                    break;
                }

                FFGTextureRef* TextureRef = static_cast<FFGTextureRef*>(WriteUsage.Resource);

                if (TextureRef->CurrentImageLayout != NewLayout || TextureRef->CurrentAccessFlags != NewAccess || TextureRef->CurrentPipelineStageFlags != NewStage)
                {
                    SetImageBarrier(CommandBuffer->GetHandle(), TextureRef->Handle->Image, TextureRef->CurrentImageLayout, NewLayout, TextureRef->CurrentAccessFlags, NewAccess, TextureRef->CurrentPipelineStageFlags, NewStage);

                    TextureRef->CurrentImageLayout = NewLayout;
                    TextureRef->CurrentAccessFlags = NewAccess;
                    TextureRef->CurrentPipelineStageFlags = NewStage;
                }

                if (NewAccess == VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT || NewAccess == VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
                {
                    ImageViews.push_back(TextureRef->View->View);
                }
            }

            if ((Pass.Flags & EFGPassFlags::Graphics) == EFGPassFlags::Graphics)
            {
                CommandList.Enqueue<FVulkanCommandBeginRendering>(ImageViews);
            }

            if (Pass.Callback)
            {
                Pass.Callback(CommandList);
            }

            if ((Pass.Flags & EFGPassFlags::Graphics) == EFGPassFlags::Graphics)
            {
                CommandList.Enqueue<FVulkanCommandEndRendering>();
            }
        }

        FRHICommandListExecutor::Execute(CommandListCtx, CommandList.Flush());
    }

    m_Device.GetGraphicsQueue()->Submit(GraphicsCommandBufferCtx);
}
