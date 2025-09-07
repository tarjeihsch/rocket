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

void FVulkanCommandListContext::RHISetScissorRect(uint32_t X, uint32_t Y)
{
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

    VkDeviceSize Offset = 0;
    VulkanRHI::vkCmdBindVertexBuffers(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &VertexBufferPtr, &Offset);
    VulkanRHI::vkCmdBindIndexBuffer(GetCurrentCommandBuffer()->GetHandle(), IndexBufferPtr, 0, VK_INDEX_TYPE_UINT32);
    VulkanRHI::vkCmdPushConstants(GetCurrentCommandBuffer()->GetHandle(), GetVulkanRHIMinimal()->GetBindlessPipelineLayout(), VK_SHADER_STAGE_ALL_GRAPHICS, 0, sizeof(FRHIDrawParams), &InDrawParams);
    VulkanRHI::vkCmdDrawIndexed(GetCurrentCommandBuffer()->GetHandle(), DrawCommand.IndexCount, 1, 0, 0, 0);
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
    ImageBarrier.image = reinterpret_cast<VkImage>(InTexture->GetHandle());

    // TODO: use a switch
    if (InTexture->RHIGetTextureDesc().Format == ERHITextureFormat::B8G8R8A8_SRGB) ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    else if (InTexture->RHIGetTextureDesc().Format == ERHITextureFormat::D32_SFLOAT) ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    else if (InTexture->RHIGetTextureDesc().Format == ERHITextureFormat::D32_SFLOAT_S8_UINT) ImageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

    ImageBarrier.subresourceRange.baseMipLevel = 0;
    ImageBarrier.subresourceRange.levelCount = 1;
    ImageBarrier.subresourceRange.baseArrayLayer = 0;
    ImageBarrier.subresourceRange.layerCount = 1;

    VkDependencyInfo DependencyInfo = {};
    DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    DependencyInfo.imageMemoryBarrierCount = 1;
    DependencyInfo.pImageMemoryBarriers = &ImageBarrier;

    VulkanRHI::vkCmdPipelineBarrier2(GetCurrentCommandBuffer()->GetHandle(), &DependencyInfo);
}

void FVulkanCommandListContext::RHIBeginRendering(std::span<FRHITextureView*> ColorAttachmentsView, FRHITextureView* DepthStencilAttachmentView)
{
    RK_ASSERT(DepthStencilAttachmentView, "DSV must point to a live object.");

    std::vector<VkRenderingAttachmentInfoKHR> AttachmentInfos = {};

    for (uint32_t Index = 0; Index < ColorAttachmentsView.size(); ++Index)
    {
        VkRenderingAttachmentInfoKHR AttachmentInfo = {};
        AttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        AttachmentInfo.imageView = static_cast<VkImageView>(ColorAttachmentsView[Index]->GetHandle());
        AttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        AttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        AttachmentInfo.clearValue.color = {{ 1, 1, 1, 1 }};
        AttachmentInfos.push_back(AttachmentInfo);
    }

    VkRenderingAttachmentInfoKHR DepthStencilAttachmentInfo{};
    DepthStencilAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    DepthStencilAttachmentInfo.imageView = static_cast<VkImageView>(DepthStencilAttachmentView->GetHandle());
    DepthStencilAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    DepthStencilAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthStencilAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthStencilAttachmentInfo.clearValue.depthStencil = { 1.0f, 0 };

    VkRenderingInfo RenderingInfo = {};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    RenderingInfo.colorAttachmentCount = AttachmentInfos.size();
    RenderingInfo.pColorAttachments = AttachmentInfos.data();
    RenderingInfo.pDepthAttachment = &DepthStencilAttachmentInfo;
    RenderingInfo.layerCount = 1;
    RenderingInfo.viewMask = 0;
    RenderingInfo.renderArea.extent.width = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().width;
    RenderingInfo.renderArea.extent.height = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().height;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().width;
    viewport.height = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().height;
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

    VulkanRHI::vkCmdCopyBuffer(GetCurrentCommandBuffer()->GetHandle(), reinterpret_cast<VkBuffer>(Src->GetHandle()), reinterpret_cast<VkBuffer>(Dst->GetHandle()), 1, &BufferCopy);
}
void FVulkanCommandListContext::RHICmdBlitImage(FRHITexture *Src, FRHITexture *Dst)
{
    VkImageBlit BlitRegion = {};
    BlitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BlitRegion.srcSubresource.mipLevel = 0;
    BlitRegion.srcSubresource.baseArrayLayer = 0;
    BlitRegion.srcSubresource.layerCount = 1;
    BlitRegion.srcOffsets[0] = { 0, 0, 0 };
    BlitRegion.srcOffsets[1] = { (int32_t)Src->RHIGetTextureDesc().Tex2D.Extent[0], (int32_t)Src->RHIGetTextureDesc().Tex2D.Extent[1], 1 };

    BlitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    BlitRegion.dstSubresource.mipLevel = 0;
    BlitRegion.dstSubresource.baseArrayLayer = 0;
    BlitRegion.dstSubresource.layerCount = 1;
    BlitRegion.dstOffsets[0] = { 0, 0, 0 };
    BlitRegion.dstOffsets[1] = { (int32_t)Dst->RHIGetTextureDesc().Tex2D.Extent[0], (int32_t)Dst->RHIGetTextureDesc().Tex2D.Extent[1], 1 };

    VulkanRHI::vkCmdBlitImage(GetCurrentCommandBuffer()->GetHandle(), reinterpret_cast<VkImage>(Src->GetHandle()), VulkanRHI::GetVulkanLayout(Src->CurrentLayout), reinterpret_cast<VkImage>(Dst->GetHandle()), VulkanRHI::GetVulkanLayout(Dst->CurrentLayout), 1, &BlitRegion, VK_FILTER_NEAREST);
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

void FVulkanCommandListContext::RHIBindVertexBuffer(VkBuffer pBuffer)
{
    VulkanRHI::vkCmdBindVertexBuffers(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &pBuffer,  nullptr); // potential issue?
}

void FVulkanCommandListContext::RHIBindIndexBuffer(VkBuffer pBuffer)
{
    VulkanRHI::vkCmdBindIndexBuffer(GetCurrentCommandBuffer()->GetHandle(), pBuffer, 0, VK_INDEX_TYPE_UINT32);
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

void FVulkanCommandListContext::RHISetScissor(int32_t p_OffsetX, int32_t p_OffsetY, uint32_t p_ExtentWidth, uint32_t p_ExtentHeight)
{
    VkRect2D Scissor;
    Scissor.offset.x = p_OffsetX;
    Scissor.offset.y = p_OffsetY;
    Scissor.extent.width = p_ExtentWidth;
    Scissor.extent.height = p_ExtentHeight;
    VulkanRHI::vkCmdSetScissor(GetCurrentCommandBuffer()->GetHandle(), 0, 1, &Scissor);
}

void FVulkanCommandListContext::RHIBindDescriptorSets(VkPipelineBindPoint p_PipelineBindPoint, VkPipelineLayout p_PipelineLayout, uint32_t p_FirstSet, uint32_t p_DescriptorSetCount, const VkDescriptorSet *p_DescriptorSets)
{
    VulkanRHI::vkCmdBindDescriptorSets(GetCurrentCommandBuffer()->GetHandle(), p_PipelineBindPoint, p_PipelineLayout, p_FirstSet, p_DescriptorSetCount, p_DescriptorSets, 0, nullptr);
}