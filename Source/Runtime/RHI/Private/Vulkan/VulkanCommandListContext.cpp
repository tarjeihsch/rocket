#include "VulkanCommandListContext.h"

#include "Assertion.h"
#include "Common/RHIBuffer.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "VulkanPipeline.h"
#include "VulkanRHI.h"
#include "VulkanSemaphore.h"
#include "Common/RHIDrawCommand.h"
#include "VulkanTexture.h"
#include "VulkanSurface.h"
#include "Math/Vector2.h"

VulkanCommandListContext::VulkanCommandListContext(VulkanDevice &in_device)
    : VulkanCommandBufferContext(*in_device.GetGraphicsQueue()->GetCommandBufferPool())
{
}

void VulkanCommandListContext::rhi_bind_pipeline(RHIPipeline *pipeline)
{
    VkDescriptorSet descriptor_set = get_vulkan_rhi_minimal()->GetBindlessDescriptorSet();

    VulkanRHI::vkCmdBindPipeline(get_current_command_buffer()->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, reinterpret_cast<VkPipeline>(pipeline->GetHandle()));
    VulkanRHI::vkCmdBindDescriptorSets(get_current_command_buffer()->get_handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, get_vulkan_rhi_minimal()->GetBindlessPipelineLayout(), 0, 1, &descriptor_set, 0, nullptr);
}

void VulkanCommandListContext::rhi_set_scissor_rect(Vector2i position, Vector2i extent)
{
    VkRect2D scissor = { position.x, position.y, (uint32_t)extent.x, (uint32_t)extent.y };

    VulkanRHI::vkCmdSetScissor(get_current_command_buffer()->get_handle(), 0, 1, &scissor);
}

void VulkanCommandListContext::rhi_pipeline_barrier_texture(RHITexture *texture, const RHITextureAccessState& from_texture_access_state, const RHITextureAccessState& to_texture_access_state)
{
    VkImageMemoryBarrier2 ImageBarrier = {};
    ImageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
    ImageBarrier.srcStageMask = VulkanRHI::GetVkPipelineStage(from_texture_access_state.pipeline_stage);
    ImageBarrier.dstStageMask = VulkanRHI::GetVkPipelineStage(to_texture_access_state.pipeline_stage);
    ImageBarrier.srcAccessMask = VulkanRHI::GetVkAccess(from_texture_access_state.resource_access);
    ImageBarrier.dstAccessMask = VulkanRHI::GetVkAccess(to_texture_access_state.resource_access);
    ImageBarrier.oldLayout = VulkanRHI::GetVulkanLayout(from_texture_access_state.texture_layout);
    ImageBarrier.newLayout = VulkanRHI::GetVulkanLayout(to_texture_access_state.texture_layout);
    ImageBarrier.srcQueueFamilyIndex = UINT32_MAX;
    ImageBarrier.dstQueueFamilyIndex = UINT32_MAX;
    ImageBarrier.image = static_cast<VkImage>(texture->GetHandle());
    ImageBarrier.subresourceRange.baseMipLevel = 0;
    ImageBarrier.subresourceRange.levelCount = 1;
    ImageBarrier.subresourceRange.baseArrayLayer = 0;
    ImageBarrier.subresourceRange.layerCount = 1;
    ImageBarrier.subresourceRange.aspectMask = VulkanRHI::GetVkAspectFromFormat(texture->GetTextureDesc().Format);

    VkDependencyInfo DependencyInfo = {};
    DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    DependencyInfo.imageMemoryBarrierCount = 1;
    DependencyInfo.pImageMemoryBarriers = &ImageBarrier;

    VulkanRHI::vkCmdPipelineBarrier2(get_current_command_buffer()->get_handle(), &DependencyInfo);
}

void VulkanCommandListContext::rhi_pipeline_barrier_buffer(RHIBuffer *buffer, const RHIBufferAccessState& from_buffer_access_state, const RHIBufferAccessState& to_buffer_access_state)
{
    VkBufferMemoryBarrier2 BufferBarrier = {};
    BufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
    BufferBarrier.pNext = nullptr;
    BufferBarrier.srcStageMask = VulkanRHI::GetVkPipelineStage(from_buffer_access_state.pipeline_stage);
    BufferBarrier.dstStageMask = VulkanRHI::GetVkPipelineStage(to_buffer_access_state.pipeline_stage);
    BufferBarrier.srcAccessMask = VulkanRHI::GetVkAccess(from_buffer_access_state.resource_access);
    BufferBarrier.dstAccessMask = VulkanRHI::GetVkAccess(to_buffer_access_state.resource_access);
    BufferBarrier.srcQueueFamilyIndex = UINT32_MAX;
    BufferBarrier.dstQueueFamilyIndex = UINT32_MAX;
    BufferBarrier.buffer = static_cast<VkBuffer>(buffer->get_handle());
    BufferBarrier.offset = 0;
    BufferBarrier.size = VK_WHOLE_SIZE;

    VkDependencyInfo DependencyInfo = {};
    DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    DependencyInfo.pNext = nullptr;
    DependencyInfo.dependencyFlags = 0;
    DependencyInfo.memoryBarrierCount = 0;
    DependencyInfo.pMemoryBarriers = nullptr;
    DependencyInfo.bufferMemoryBarrierCount = 1;
    DependencyInfo.pBufferMemoryBarriers = &BufferBarrier;
    DependencyInfo.imageMemoryBarrierCount = 0;
    DependencyInfo.pImageMemoryBarriers = nullptr;

    VulkanRHI::vkCmdPipelineBarrier2(get_current_command_buffer()->get_handle(), &DependencyInfo);
}

void VulkanCommandListContext::rhi_begin_rendering(std::span<RHITextureView*> color_attachment_view, RHITextureView* depth_stencil_attachment_view, bool bClearColor)
{
    std::vector<VkRenderingAttachmentInfoKHR> AttachmentInfos = {};

    for (uint32_t Index = 0; Index < color_attachment_view.size(); ++Index)
    {
        VkRenderingAttachmentInfoKHR AttachmentInfo = {};
        AttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        AttachmentInfo.imageView = static_cast<VkImageView>(color_attachment_view[Index]->get_handle());
        AttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        AttachmentInfo.loadOp = bClearColor ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
        AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        AttachmentInfo.clearValue.color = {{ 0, 1, 0, 1 }};
        AttachmentInfos.push_back(AttachmentInfo);
    }

    RHITexture* texture = color_attachment_view[0]->get_target_texture();
    Vector2i extent = Vector2i(texture->GetTextureDesc().extent[0], texture->GetTextureDesc().extent[1]);

    VkRenderingInfo rendering_info = {};
    rendering_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
    rendering_info.colorAttachmentCount = AttachmentInfos.size();
    rendering_info.pColorAttachments = AttachmentInfos.data();
    rendering_info.layerCount = 1;
    rendering_info.viewMask = 0;
    rendering_info.renderArea.extent.width = extent.x;
    rendering_info.renderArea.extent.height = extent.y;

    if (depth_stencil_attachment_view)
    {
        VkRenderingAttachmentInfoKHR depth_stencil_attachment_info{};
        depth_stencil_attachment_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depth_stencil_attachment_info.imageView = static_cast<VkImageView>(depth_stencil_attachment_view->get_handle());
        depth_stencil_attachment_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depth_stencil_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_stencil_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_stencil_attachment_info.clearValue.depthStencil = { 1.0f, 0 };
        rendering_info.pDepthAttachment = &depth_stencil_attachment_info;
    }

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.x);
    viewport.height = static_cast<float>(extent.y);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = { 0, 0 };
    scissor.extent = { static_cast<uint32_t>(extent.x), static_cast<uint32_t>(extent.y) };

    VulkanRHI::vkCmdBeginRendering(get_current_command_buffer()->get_handle(), &rendering_info);
    VulkanRHI::vkCmdSetViewport(get_current_command_buffer()->get_handle(), 0, 1, &viewport);
    VulkanRHI::vkCmdSetScissor(get_current_command_buffer()->get_handle(), 0, 1, &scissor);
}

void VulkanCommandListContext::rhi_end_rendering()
{
    VulkanRHI::vkCmdEndRendering(get_current_command_buffer()->get_handle());
}

void VulkanCommandListContext::rhi_copy_buffer(RHIBuffer *Src, RHIBuffer *Dst, uint32_t InSrcOffset, uint32_t InDstOffset, uint32_t InSize)
{
    VkBufferCopy buffer_copy = {};
    buffer_copy.srcOffset = 0;
    buffer_copy.dstOffset = 0;
    buffer_copy.size = Src->desc.size;

    VulkanRHI::vkCmdCopyBuffer(get_current_command_buffer()->get_handle(), static_cast<VkBuffer>(Src->get_handle()), static_cast<VkBuffer>(Dst->get_handle()), 1, &buffer_copy);
}

void VulkanCommandListContext::rhi_copy_image(RHITexture *Src, RHITexture *Dst)
{
    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcOffsets[0] = { 0, 0, 0 };
    blit_region.srcOffsets[1] = { (int32_t)Src->GetTextureDesc().extent[0], (int32_t)Src->GetTextureDesc().extent[1], 1 };
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.mipLevel = 0;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstOffsets[0] = { 0, 0, 0 };
    blit_region.dstOffsets[1] = { (int32_t)Dst->GetTextureDesc().extent[0], (int32_t)Dst->GetTextureDesc().extent[1], 1 };

    VulkanRHI::vkCmdBlitImage(get_current_command_buffer()->get_handle(), reinterpret_cast<VkImage>(Src->GetHandle()), VulkanRHI::GetVulkanLayout(Src->GetTextureAccessState().texture_layout), reinterpret_cast<VkImage>(Dst->GetHandle()), VulkanRHI::GetVulkanLayout(Dst->GetTextureAccessState().texture_layout), 1, &blit_region, VK_FILTER_NEAREST);
}