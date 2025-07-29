#include "VulkanCommandList.h"

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanLoader.h"

void FVulkanCommandListContext::RHISetScissorRect(uint32_t X, uint32_t Y)
{
}

void FVulkanCommandListContext::RHIBeginRendering(std::span<VkImageView> ImageViews)
{
    std::vector<VkRenderingAttachmentInfoKHR> ColorAttachmentInfos(ImageViews.size());

    for (uint32_t Index = 0; Index < ImageViews.size(); ++Index)
    {
        ColorAttachmentInfos[Index].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        ColorAttachmentInfos[Index].imageView        = ImageViews[Index];
        ColorAttachmentInfos[Index].imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        ColorAttachmentInfos[Index].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        ColorAttachmentInfos[Index].storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        ColorAttachmentInfos[Index].clearValue.color = {{ 0, 0, 0, 1 }};
    }

    VkRenderingInfo RenderingInfo = {};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    RenderingInfo.colorAttachmentCount = ColorAttachmentInfos.size();
    RenderingInfo.pColorAttachments    = ColorAttachmentInfos.data();
    RenderingInfo.layerCount           = 1;
    RenderingInfo.viewMask             = 0;
    RenderingInfo.renderArea.extent.width  = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().width;
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

    VulkanRHI::vkCmdBeginRendering(CommandBuffer, &RenderingInfo);
    VulkanRHI::vkCmdSetViewport(CommandBuffer, 0, 1, &viewport);
    VulkanRHI::vkCmdSetScissor(CommandBuffer, 0, 1, &scissor);
}

void FVulkanCommandListContext::RHIEndRendering()
{
    VulkanRHI::vkCmdEndRendering(CommandBuffer);
}

void FVulkanCommandListContext::RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout, VkAccessFlags p_SrcAccess, VkAccessFlags p_DstAccess,  VkPipelineStageFlags2 SrcStage, VkPipelineStageFlags2 DstStage, uint32_t p_SrcQueueFamilyIndex, uint32_t p_DstQueueFamilyIndex)
{
    VkImageMemoryBarrier ImageMemoryBarrier = {};
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImageMemoryBarrier.oldLayout = OldLayout;
    ImageMemoryBarrier.newLayout = NewLayout;
    ImageMemoryBarrier.srcQueueFamilyIndex = p_SrcQueueFamilyIndex;
    ImageMemoryBarrier.dstQueueFamilyIndex = p_DstQueueFamilyIndex;
    ImageMemoryBarrier.image = Image;
    ImageMemoryBarrier.srcAccessMask = p_SrcAccess;
    ImageMemoryBarrier.dstAccessMask = p_DstAccess;
    ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    ImageMemoryBarrier.subresourceRange.levelCount = 1;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    ImageMemoryBarrier.subresourceRange.layerCount = 1;

    VulkanRHI::vkCmdPipelineBarrier(CommandBuffer, SrcStage, DstStage, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
}

void FVulkanCommandListContext::RHIBindPipeline(VkPipelineBindPoint pPipelineBindPoint, VkPipeline pPipeline)
{
    VulkanRHI::vkCmdBindPipeline(CommandBuffer, pPipelineBindPoint, pPipeline);
}

void FVulkanCommandListContext::RHIBindVertexBuffer(VkBuffer pBuffer)
{
    VulkanRHI::vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &pBuffer,  nullptr); // potential issue?
}

void FVulkanCommandListContext::RHIBindIndexBuffer(VkBuffer pBuffer)
{
    VulkanRHI::vkCmdBindIndexBuffer(CommandBuffer, pBuffer, 0, VK_INDEX_TYPE_UINT32);
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
    VulkanRHI::vkCmdSetViewport(CommandBuffer, 0, 1, &Viewport);
}

void FVulkanCommandListContext::RHISetScissor(int32_t p_OffsetX, int32_t p_OffsetY, uint32_t p_ExtentWidth, uint32_t p_ExtentHeight)
{
    VkRect2D Scissor;
    Scissor.offset.x = p_OffsetX;
    Scissor.offset.y = p_OffsetY;
    Scissor.extent.width = p_ExtentWidth;
    Scissor.extent.height = p_ExtentHeight;
    VulkanRHI::vkCmdSetScissor(CommandBuffer, 0, 1, &Scissor);
}

void FVulkanCommandListContext::RHIBindDescriptorSets(VkPipelineBindPoint p_PipelineBindPoint, VkPipelineLayout p_PipelineLayout, uint32_t p_FirstSet, uint32_t p_DescriptorSetCount, const VkDescriptorSet *p_DescriptorSets)
{
    VulkanRHI::vkCmdBindDescriptorSets(CommandBuffer, p_PipelineBindPoint, p_PipelineLayout, p_FirstSet, p_DescriptorSetCount, p_DescriptorSets, 0, nullptr);
}

void FVulkanCommandListContext::RHICopyBuffer(VkBuffer p_StagingBuffer, VkBuffer p_Buffer)
{
    VkBufferCopy BufferCopy = {};
    BufferCopy.srcOffset = 0;
    BufferCopy.dstOffset = 0;
    BufferCopy.size = 64ull * 1024 * 1024;

    VulkanRHI::vkCmdCopyBuffer(CommandBuffer, p_StagingBuffer, p_Buffer, 1, &BufferCopy);
}
