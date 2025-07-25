#include "VulkanCommandList.h"

#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanLoader.h"

void FVulkanCommandListContext::RHISetScissorRect(uint32_t X, uint32_t Y)
{
}

void FVulkanCommandListContext::RHIBeginRendering(VkImageView ImageView)
{
    VkRenderingAttachmentInfoKHR ColorAttachmentInfo = {};
    ColorAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    ColorAttachmentInfo.imageView        = ImageView;
    ColorAttachmentInfo.imageLayout      = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Fixed layout
    ColorAttachmentInfo.loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachmentInfo.storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachmentInfo.clearValue.color = {{ 1, 1, 0, 1 }};

    VkRenderingInfo RenderingInfo = {};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    RenderingInfo.colorAttachmentCount = 1;
    RenderingInfo.pColorAttachments    = &ColorAttachmentInfo;
    RenderingInfo.layerCount           = 1;
    RenderingInfo.viewMask             = 0;
    RenderingInfo.renderArea.extent.width  = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().width;
    RenderingInfo.renderArea.extent.height = GetVulkanRHIMinimal()->RHIGetSwapchainExtent().height;

    VulkanRHI::vkCmdBeginRendering(CommandBuffer, &RenderingInfo);
}

void FVulkanCommandListContext::RHIEndRendering()
{
    VulkanRHI::vkCmdEndRendering(CommandBuffer);
}

void FVulkanCommandListContext::RHISetMemoryBarrier(VkImage Image, VkImageLayout OldLayout, VkImageLayout NewLayout)
{
    VkImageMemoryBarrier ImageMemoryBarrier = {};
    ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    ImageMemoryBarrier.oldLayout = OldLayout;
    ImageMemoryBarrier.newLayout = NewLayout;
    ImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    ImageMemoryBarrier.image = Image;
    ImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    ImageMemoryBarrier.subresourceRange.levelCount = 1;
    ImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    ImageMemoryBarrier.subresourceRange.layerCount = 1;
    ImageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    ImageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    
    VulkanRHI::vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &ImageMemoryBarrier);
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
