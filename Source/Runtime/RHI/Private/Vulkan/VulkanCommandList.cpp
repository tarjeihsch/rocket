#include "VulkanCommandList.h"

#include "Assert.h"
#include "Common/RHIBuffer.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"

FVulkanCommandListContext::FVulkanCommandListContext(FVulkanDevice &InDevice)
    : FVulkanCommandBufferContext(*InDevice.GetGraphicsQueue()->GetCommandBufferPool())
{
}

void FVulkanCommandListContext::RHIBindPipeline(FRHIPipeline *Pipeline)
{
    VkDescriptorSet DS = GetVulkanRHIMinimal()->GetBindlessDescriptorSet();

    VulkanRHI::vkCmdBindPipeline(GetCurrentCommandBuffer()->GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, reinterpret_cast<VkPipeline>(Pipeline->GetHandle()));
    VulkanRHI::vkCmdBindDescriptorSets(GetCurrentCommandBuffer()->GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, GetVulkanRHIMinimal()->GetBindlessPipelineLayout(), 0, 1, &DS, 0, nullptr);
}

void FVulkanCommandListContext::RHISetScissorRect(int32_t X, int32_t Y, uint32_t Width, uint32_t Height)
{
    VkRect2D Scissor = { X, Y, Width, Height };
    VulkanRHI::vkCmdSetScissor(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &Scissor);
}

void FVulkanCommandListContext::RHIDraw(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& Params)
{
    VkBuffer VertexBufferPtr = reinterpret_cast<VkBuffer>(DrawCommand.VertexBuffer->GetHandle());

    VkDeviceSize Offset = { 0 };
    VulkanRHI::vkCmdBindVertexBuffers(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &VertexBufferPtr, &Offset);
    VulkanRHI::vkCmdPushConstants(GetCurrentCommandBuffer()->GetHandle(), GetVulkanRHIMinimal()->GetBindlessPipelineLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(FRHIDrawParams), &Params);
    VulkanRHI::vkCmdDraw(GetCurrentCommandBuffer()->GetHandle(), DrawCommand.IndexCount, 1, 0, 0);
}

void FVulkanCommandListContext::RHIDrawIndexed(const FRHIDrawCommand& DrawCommand, const FRHIDrawParams& InDrawParams)
{
    VkBuffer VertexBufferPtr = reinterpret_cast<VkBuffer>(DrawCommand.VertexBuffer->GetHandle());
    VkBuffer IndexBufferPtr = reinterpret_cast<VkBuffer>(DrawCommand.IndexBuffer->GetHandle());

    VulkanRHI::vkCmdBindVertexBuffers(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &VertexBufferPtr, (VkDeviceSize[]){0});
    VulkanRHI::vkCmdBindIndexBuffer(GetCurrentCommandBuffer()->GetHandle(), IndexBufferPtr, 0, DrawCommand.IndexType == FRHIDrawCommand::EIndexType::U32 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
    VulkanRHI::vkCmdPushConstants(GetCurrentCommandBuffer()->GetHandle(), GetVulkanRHIMinimal()->GetBindlessPipelineLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(FRHIDrawParams), &InDrawParams);
    VulkanRHI::vkCmdDrawIndexed(GetCurrentCommandBuffer()->GetHandle(), DrawCommand.IndexCount, 1, DrawCommand.IndexBufferOffset, static_cast<int32_t>(DrawCommand.VertexBufferOffset), 0);
}

void FVulkanCommandListContext::RHIPipelineBarrier(FRHITexture *InTexture, ERHITextureLayout InOldLayout, ERHITextureLayout InNewLayout, ERHIResourceAccess InSrcAccess, ERHIResourceAccess InDstAccess, ERHIPipelineStage InSrcStage, ERHIPipelineStage InDstStage, uint32_t InSrcQueueFamilyIndex, uint32_t InDstQueueFamilyIndex)
{
    VkImageMemoryBarrier2 ImageBarrier = {};
    ImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    ImageBarrier.srcStageMask = VulkanRHI::GetVulkanPipelineStage(InSrcStage);
    ImageBarrier.dstStageMask = VulkanRHI::GetVulkanPipelineStage(InDstStage);
    ImageBarrier.srcAccessMask = VulkanRHI::GetVulkanAccess(InSrcAccess);
    ImageBarrier.dstAccessMask = VulkanRHI::GetVulkanAccess(InDstAccess);
    ImageBarrier.oldLayout = VulkanRHI::GetVulkanLayout(InOldLayout);
    ImageBarrier.newLayout = VulkanRHI::GetVulkanLayout(InNewLayout);
    ImageBarrier.srcQueueFamilyIndex = InSrcQueueFamilyIndex;
    ImageBarrier.dstQueueFamilyIndex = InDstQueueFamilyIndex;
    ImageBarrier.image = static_cast<VkImage>(InTexture->GetHandle());
    ImageBarrier.subresourceRange.baseMipLevel = 0;
    ImageBarrier.subresourceRange.levelCount = 1;
    ImageBarrier.subresourceRange.baseArrayLayer = 0;
    ImageBarrier.subresourceRange.layerCount = 1;

    switch (InTexture->RHIGetTextureDesc().Format)
    {
    case ERHITextureFormat::B8G8R8A8_SRGB:
    case ERHITextureFormat::R8G8B8A8_UNORM:
        ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        break;
    case ERHITextureFormat::D32_SFLOAT:
        ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        break;
    case ERHITextureFormat::D32_SFLOAT_S8_UINT:
        ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        break;
    default:
        break;
    }

    VkDependencyInfo DependencyInfo = {};
    DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    DependencyInfo.imageMemoryBarrierCount = 1;
    DependencyInfo.pImageMemoryBarriers = &ImageBarrier;

    VulkanRHI::vkCmdPipelineBarrier2(GetCurrentCommandBuffer()->GetHandle(), &DependencyInfo);
}

void FVulkanCommandListContext::RHIBeginRendering(std::span<FRHITextureView*> ColorAttachmentsView, FRHITextureView* DepthStencilAttachmentView, bool bClearColor)
{
    std::vector<VkRenderingAttachmentInfoKHR> AttachmentInfos = {};

    for (uint32_t Index = 0; Index < ColorAttachmentsView.size(); ++Index)
    {
        VkRenderingAttachmentInfoKHR AttachmentInfo = {};
        AttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        AttachmentInfo.imageView = static_cast<VkImageView>(ColorAttachmentsView[Index]->GetHandle());
        AttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        AttachmentInfo.loadOp = bClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        AttachmentInfo.clearValue.color = {{ 1, 1, 1, 1 }};
        AttachmentInfos.push_back(AttachmentInfo);
    }

    VkRenderingInfo RenderingInfo = {};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    RenderingInfo.colorAttachmentCount = AttachmentInfos.size();
    RenderingInfo.pColorAttachments = AttachmentInfos.data();
    RenderingInfo.layerCount = 1;
    RenderingInfo.viewMask = 0;
    RenderingInfo.renderArea.extent.width = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().width;
    RenderingInfo.renderArea.extent.height = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().height;

    if (DepthStencilAttachmentView)
    {
        VkRenderingAttachmentInfoKHR DepthStencilAttachmentInfo{};
        DepthStencilAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        DepthStencilAttachmentInfo.imageView = static_cast<VkImageView>(DepthStencilAttachmentView->GetHandle());
        DepthStencilAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        DepthStencilAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        DepthStencilAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        DepthStencilAttachmentInfo.clearValue.depthStencil = { 1.0f, 0 };
        RenderingInfo.pDepthAttachment = &DepthStencilAttachmentInfo;
    }

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(GetVulkanRHIMinimal()->RHIGetSwapchainExtent().width);
    viewport.height = static_cast<float>(GetVulkanRHIMinimal()->RHIGetSwapchainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = GetVulkanRHIMinimal()->RHIGetSwapchainExtent();

    VulkanRHI::vkCmdBeginRendering(GetCurrentCommandBuffer()->GetHandle(), &RenderingInfo);
    VulkanRHI::vkCmdSetViewport(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &viewport);
    VulkanRHI::vkCmdSetScissor(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &scissor);
}

void FVulkanCommandListContext::RHIEndRendering()
{
    VulkanRHI::vkCmdEndRendering(GetCurrentCommandBuffer()->GetHandle());
}

void FVulkanCommandListContext::RHICopyBuffer(FRHIBuffer *Src, FRHIBuffer *Dst, uint32_t InSrcOffset, uint32_t InDstOffset, uint32_t InSize)
{
    VkBufferCopy BufferCopy = {};
    BufferCopy.srcOffset = 0;
    BufferCopy.dstOffset = 0;
    BufferCopy.size = Src->GetBufferDesc().Size;

    VulkanRHI::vkCmdCopyBuffer(GetCurrentCommandBuffer()->GetHandle(), static_cast<VkBuffer>(Src->GetHandle()), static_cast<VkBuffer>(Dst->GetHandle()), 1, &BufferCopy);
}

void FVulkanCommandListContext::RHICmdBlitImage(FRHITexture *Src, FRHITexture *Dst)
{
    VkImageBlit BlitRegion = {};
    BlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BlitRegion.srcSubresource.mipLevel = 0;
    BlitRegion.srcSubresource.baseArrayLayer = 0;
    BlitRegion.srcSubresource.layerCount = 1;
    BlitRegion.srcOffsets[0] = { 0, 0, 0 };
    BlitRegion.srcOffsets[1] = { (int32_t)Src->RHIGetTextureDesc().Extent[0], (int32_t)Src->RHIGetTextureDesc().Extent[1], 1 };

    BlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BlitRegion.dstSubresource.mipLevel = 0;
    BlitRegion.dstSubresource.baseArrayLayer = 0;
    BlitRegion.dstSubresource.layerCount = 1;
    BlitRegion.dstOffsets[0] = { 0, 0, 0 };
    BlitRegion.dstOffsets[1] = { (int32_t)Dst->RHIGetTextureDesc().Extent[0], (int32_t)Dst->RHIGetTextureDesc().Extent[1], 1 };

    VulkanRHI::vkCmdBlitImage(GetCurrentCommandBuffer()->GetHandle(), reinterpret_cast<VkImage>(Src->GetHandle()), VulkanRHI::GetVulkanLayout(Src->CurrentLayout), reinterpret_cast<VkImage>(Dst->GetHandle()), VulkanRHI::GetVulkanLayout(Dst->CurrentLayout), 1, &BlitRegion, VK_FILTER_NEAREST);
}

void FVulkanCommandListContext::RHICopyBufferToImage(FRHIBuffer *SrcBuffer, FRHITexture *DstTexture, VkImageLayout DstImageLayout, uint32_t Width, uint32_t Height, uint32_t MipLevel, uint32_t BaseArrayLayer, uint32_t LayerCount, uint64_t BufferOffset)
{
    VkBuffer SrcBufferPtr = static_cast<VkBuffer>(SrcBuffer->GetHandle());
    VkImage DstImagePtr = static_cast<VkImage>(DstTexture->GetHandle());

    VkBufferImageCopy Buffer = {};
    Buffer.bufferOffset = BufferOffset;
    Buffer.bufferRowLength = 0;
    Buffer.bufferImageHeight = 0;
    Buffer.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Buffer.imageSubresource.mipLevel = MipLevel;
    Buffer.imageSubresource.baseArrayLayer = BaseArrayLayer;
    Buffer.imageSubresource.layerCount = LayerCount;
    Buffer.imageOffset = { 0, 0, 0 };
    Buffer.imageExtent = { Width, Height, 1 };

    VulkanRHI::vkCmdCopyBufferToImage(GetCurrentCommandBuffer()->GetHandle(), SrcBufferPtr, DstImagePtr, DstImageLayout, 1, &Buffer);
}

void FVulkanCommandListContext::RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout, VkAccessFlags p_SrcAccess, VkAccessFlags p_DstAccess,  VkPipelineStageFlags2 SrcStage, VkPipelineStageFlags2 DstStage, uint32_t p_SrcQueueFamilyIndex, uint32_t p_DstQueueFamilyIndex)
{
    VkImageMemoryBarrier2 ImageMemoryBarrier = {};
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    ImageMemoryBarrier.pNext = nullptr;
    ImageMemoryBarrier.srcStageMask = SrcStage;
    ImageMemoryBarrier.srcAccessMask = p_SrcAccess;
    ImageMemoryBarrier.dstStageMask = DstStage;
    ImageMemoryBarrier.dstAccessMask = p_DstAccess;
    ImageMemoryBarrier.oldLayout = OldLayout;
    ImageMemoryBarrier.newLayout = NewLayout;
    ImageMemoryBarrier.srcQueueFamilyIndex = p_SrcQueueFamilyIndex;
    ImageMemoryBarrier.dstQueueFamilyIndex = p_DstQueueFamilyIndex;
    ImageMemoryBarrier.image = Image;
    ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    ImageMemoryBarrier.subresourceRange.levelCount = 1;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    ImageMemoryBarrier.subresourceRange.layerCount = 1;

    VkDependencyInfo dependencyInfo = {};
    dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    dependencyInfo.pNext = nullptr;
    dependencyInfo.dependencyFlags = 0;
    dependencyInfo.memoryBarrierCount = 0;
    dependencyInfo.pMemoryBarriers = nullptr;
    dependencyInfo.bufferMemoryBarrierCount = 0;
    dependencyInfo.pBufferMemoryBarriers = nullptr;
    dependencyInfo.imageMemoryBarrierCount = 1;
    dependencyInfo.pImageMemoryBarriers = &ImageMemoryBarrier;

    VulkanRHI::vkCmdPipelineBarrier2(GetCurrentCommandBuffer()->GetHandle(), &dependencyInfo);
}

void FVulkanCommandListContext::RHISetViewport(float p_X, float p_Y, float p_Width, float p_Height, float p_MinDepth, float p_MaxDepth)
{
    VkViewport Viewport;
    Viewport.x = p_X;
    Viewport.y = p_Y;
    Viewport.width = p_Width;
    Viewport.height = p_Height;
    Viewport.minDepth = p_MinDepth;
    Viewport.maxDepth = p_MaxDepth;
    VulkanRHI::vkCmdSetViewport(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &Viewport);
}