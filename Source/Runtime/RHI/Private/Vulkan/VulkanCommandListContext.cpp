#include "VulkanCommandListContext.h"

#include "Assertion.h"
#include "VulkanBarrier.h"
#include "Common/RHIBuffer.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "VulkanPipeline.h"
#include "VulkanSemaphore.h"
#include "VulkanSurface.h"
#include "Math/Vector2.h"
#include <Common/RHITextureView.h>

VulkanCommandListContext::VulkanCommandListContext(VulkanDevice &in_device)
    : Device(in_device), VulkanCommandBufferContext(*in_device.GetGraphicsQueue()->GetCommandBufferPool())
{
}

void VulkanCommandListContext::RHIBindGlobalHeap()
{
    VkDescriptorSet Sets[] = {
        Device.GetBindlessDescriptorManager()->GetHeap().GetDescriptorSet(),
        Device.GetBindlessDescriptorManager()->GetParameterBuffer().GetDescriptorSet()
    };

    VulkanRHI::vkCmdBindDescriptorSets(GetCurrentCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Device.GetBindlessDescriptorManager()->GetGlobalPipelineLayout(), 0, 2, Sets, 0, nullptr);
}

void VulkanCommandListContext::RHIBindPipeline(RHIPipeline *pipeline)
{
    VulkanRHI::vkCmdBindPipeline(GetCurrentCommandBuffer()->GetVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, reinterpret_cast<VkPipeline>(pipeline->GetHandle()));
}

void VulkanCommandListContext::RHISetScissorRect(Vector2i position, Vector2i extent)
{
    VkRect2D scissor = { position.x, position.y, static_cast<uint32_t>(extent.x), static_cast<uint32_t>(extent.y) };

    VulkanRHI::vkCmdSetScissor(GetCurrentCommandBuffer()->GetVkCommandBuffer(), 0, 1, &scissor);
}

void VulkanCommandListContext::RHISetViewport(Vector2i position, Vector2i extent)
{
    VkViewport Viewport = {};
    Viewport.x = position.x;
    Viewport.y = position.y;
    Viewport.width = extent.x;
    Viewport.height = extent.y;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VulkanRHI::vkCmdSetViewport(GetCurrentCommandBuffer()->GetVkCommandBuffer(), 0, 1, &Viewport);
}

void VulkanCommandListContext::RHIPipelineBarrierTexture(RHITexture *texture, const RHITextureAccessState& from_texture_access_state, const RHITextureAccessState& to_texture_access_state)
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
    ImageBarrier.image = static_cast<VkImage>(texture->GetNativeTexture());
    ImageBarrier.subresourceRange.baseMipLevel = 0;
    ImageBarrier.subresourceRange.levelCount = 1;
    ImageBarrier.subresourceRange.baseArrayLayer = 0;
    ImageBarrier.subresourceRange.layerCount = 1;
    ImageBarrier.subresourceRange.aspectMask = VulkanRHI::GetVkAspectFromFormat(texture->GetTextureDesc().format);

    VkDependencyInfo DependencyInfo = {};
    DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    DependencyInfo.imageMemoryBarrierCount = 1;
    DependencyInfo.pImageMemoryBarriers = &ImageBarrier;

    VulkanRHI::vkCmdPipelineBarrier2(GetCurrentCommandBuffer()->GetVkCommandBuffer(), &DependencyInfo);
}

void VulkanCommandListContext::RHIPipelineBarrierBuffer(RHIBuffer *buffer, const RHIBufferAccessState& from_buffer_access_state, const RHIBufferAccessState& to_buffer_access_state)
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
    BufferBarrier.buffer = static_cast<VkBuffer>(buffer->GetNativeBuffer());
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

    VulkanRHI::vkCmdPipelineBarrier2(GetCurrentCommandBuffer()->GetVkCommandBuffer(), &DependencyInfo);
}

void VulkanCommandListContext::RHIBeginRendering(const RHIRenderPassInfo& Info)
{
    VkRenderingAttachmentInfoKHR VkColorAttachments[MaxSimultaneousRenderTargets];
	for (uint32_t i = 0; i < Info.NumColorRenderTargets; ++i)
	{
		const RHIRenderTargetView& RT = Info.ColorRenderTargets[i];
		VkRenderingAttachmentInfoKHR& Out = VkColorAttachments[i];
		Out = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };

		if (RT.IsValid())
		{
			Out.imageView = reinterpret_cast<VkImageView>(RT.TextureView->GetNativeHandle());
			Out.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			Out.loadOp = (RT.LoadOp == LoadAction::Clear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : (RT.LoadOp == LoadAction::Load) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			Out.storeOp = (RT.StoreOp == StoreAction::Store) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

			Out.clearValue.color.float32[0] = RT.ClearValue.R;
			Out.clearValue.color.float32[1] = RT.ClearValue.G;
			Out.clearValue.color.float32[2] = RT.ClearValue.B;
			Out.clearValue.color.float32[3] = RT.ClearValue.A;
		}
	}

	VkRenderingAttachmentInfoKHR VkDepthAttachment = { VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR };
	bool bHasDepth = Info.DepthStencilRenderTarget.IsValid();

	if (bHasDepth)
	{
		const RHIDepthStencilView& DS = Info.DepthStencilRenderTarget;
		VkDepthAttachment.imageView = reinterpret_cast<VkImageView>(DS.TextureView->GetNativeHandle());
		VkDepthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkDepthAttachment.loadOp = (DS.DepthLoadOp == LoadAction::Clear) ? VK_ATTACHMENT_LOAD_OP_CLEAR : (DS.DepthLoadOp == LoadAction::Load) ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		VkDepthAttachment.storeOp = (DS.DepthStoreOp == StoreAction::Store) ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;

		VkDepthAttachment.clearValue.depthStencil.depth = DS.ClearDepth;
		VkDepthAttachment.clearValue.depthStencil.stencil = DS.ClearStencil;
	}

    VkRenderingInfoKHR RenderingInfo = {};
    RenderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
	if (Info.NumColorRenderTargets > 0)
	{
		RenderingInfo.renderArea = { {0, 0}, {1280, 720} };
	}
	else if (bHasDepth)
	{
		RenderingInfo.renderArea = { {0, 0}, {1280, 720} };
	}

	RenderingInfo.layerCount = 1;
	RenderingInfo.colorAttachmentCount = Info.NumColorRenderTargets;
	RenderingInfo.pColorAttachments = VkColorAttachments;
	RenderingInfo.pDepthAttachment = bHasDepth ? &VkDepthAttachment : nullptr;
	RenderingInfo.pStencilAttachment = nullptr; // Add if using stencil

	// Call the Vulkan function
	VulkanRHI::vkCmdBeginRendering(GetCurrentCommandBuffer()->GetVkCommandBuffer(), &RenderingInfo);
}

void VulkanCommandListContext::RHIEndRendering()
{
    VulkanRHI::vkCmdEndRendering(GetCurrentCommandBuffer()->GetVkCommandBuffer());
}

void VulkanCommandListContext::RHIAddWaitSemaphore(RHISemaphore* semaphore, RHIPipelineStage pipeline_stage)
{
    AddWaitSemaphore(static_cast<VkSemaphore>(semaphore->GetNativeHandle()), VulkanRHI::GetVkPipelineStage(pipeline_stage));
}

void VulkanCommandListContext::RHIAddSignalSemaphore(RHISemaphore* semaphore, RHIPipelineStage pipeline_stage)
{
    AddSignalSemaphore(static_cast<VkSemaphore>(semaphore->GetNativeHandle()), VulkanRHI::GetVkPipelineStage(pipeline_stage));
}

void VulkanCommandListContext::RHIBindVertexBuffer(RHIBuffer *rhi_buffer)
{
    VkBuffer native_buffer = static_cast<VkBuffer>(rhi_buffer->GetNativeBuffer());
    VulkanRHI::vkCmdBindVertexBuffers(GetCurrentCommandBuffer()->GetVkCommandBuffer(), 0, 1, &native_buffer, nullptr);
}

void VulkanCommandListContext::RHIBindIndexBuffer(RHIBuffer *rhi_buffer)
{
    VkBuffer native_buffer = static_cast<VkBuffer>(rhi_buffer->GetNativeBuffer());
    VulkanRHI::vkCmdBindIndexBuffer(GetCurrentCommandBuffer()->GetVkCommandBuffer(), native_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandListContext::RHISetShaderParameters(const void* Data, uint32_t Size)
{
    uint32_t DataOffset = Device.GetBindlessDescriptorManager()->GetParameterBuffer().Allocate(Data, Size);

    VulkanRHI::vkCmdPushConstants(GetCurrentCommandBuffer()->GetVkCommandBuffer(), Device.GetBindlessDescriptorManager()->GetGlobalPipelineLayout(), VK_SHADER_STAGE_ALL, 0, sizeof(uint32_t), &DataOffset);
}

void VulkanCommandListContext::RHIDebugMarkTimelineBegin(const ANSICHAR *Name)
{
	VkDebugUtilsLabelEXT DebugLabel = {};
	DebugLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	DebugLabel.pLabelName = Name;
	VulkanRHI::vkCmdBeginDebugUtilsLabelEXT(GetCurrentCommandBuffer()->GetVkCommandBuffer(), &DebugLabel);
}

void VulkanCommandListContext::RHIDebugMarkTimelineEnd()
{
	VkDebugUtilsLabelEXT DebugLabel = {};
	DebugLabel.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	VulkanRHI::vkCmdEndDebugUtilsLabelEXT(GetCurrentCommandBuffer()->GetVkCommandBuffer());
}

void VulkanCommandListContext::RHICopyBuffer(RHIBuffer *src_buffer, RHIBuffer *dst_buffer, uint32_t src_offset, uint32_t dst_offset, uint32_t size)
{
    VkBufferCopy buffer_copy = {};
    buffer_copy.srcOffset = 0;
    buffer_copy.dstOffset = 0;
    buffer_copy.size = src_buffer->GetBufferDesc().size;

    VulkanRHI::vkCmdCopyBuffer(GetCurrentCommandBuffer()->GetVkCommandBuffer(), static_cast<VkBuffer>(src_buffer->GetNativeBuffer()), static_cast<VkBuffer>(dst_buffer->GetNativeBuffer()), 1, &buffer_copy);
}

void VulkanCommandListContext::RHICopyImage(RHITexture *texture_src, RHITexture *texture_dst)
{
    VkImageBlit blit_region = {};
    blit_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.srcSubresource.mipLevel = 0;
    blit_region.srcSubresource.baseArrayLayer = 0;
    blit_region.srcSubresource.layerCount = 1;
    blit_region.srcOffsets[0] = { 0, 0, 0 };
    blit_region.srcOffsets[1] = { static_cast<int32_t>(texture_src->GetTextureDesc().extent[0]), static_cast<int32_t>(texture_src->GetTextureDesc().extent[1]), 1 };
    blit_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    blit_region.dstSubresource.mipLevel = 0;
    blit_region.dstSubresource.baseArrayLayer = 0;
    blit_region.dstSubresource.layerCount = 1;
    blit_region.dstOffsets[0] = { 0, 0, 0 };
    blit_region.dstOffsets[1] = { static_cast<int32_t>(texture_dst->GetTextureDesc().extent[0]), static_cast<int32_t>(texture_dst->GetTextureDesc().extent[1]), 1 };

    VulkanRHI::vkCmdBlitImage(GetCurrentCommandBuffer()->GetVkCommandBuffer(), reinterpret_cast<VkImage>(texture_src->GetNativeTexture()), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, reinterpret_cast<VkImage>(texture_dst->GetNativeTexture()), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit_region, VK_FILTER_NEAREST);
}

void VulkanCommandListContext::RHIDraw(uint32_t VertexCount, uint32_t InstanceCount, uint32_t FirstVertex, uint32_t FirstInstance)
{
    VulkanRHI::vkCmdDraw(GetCurrentCommandBuffer()->GetVkCommandBuffer(), VertexCount, InstanceCount, FirstVertex, FirstInstance);
}

void VulkanCommandListContext::RHIDrawIndexed(uint32_t index_count)
{
    VulkanRHI::vkCmdDrawIndexed(GetCurrentCommandBuffer()->GetVkCommandBuffer(), index_count, 1, 0, 0, 0);
}

void VulkanCommandListContext::RHIDrawIndirectCount(RHIBuffer *indirect_draw_command_buffer, size_t offset,
                                                    RHIBuffer *count_buffer, size_t count_offset, uint32_t max_draw_count, uint32_t stride)
{
    VulkanRHI::vkCmdDrawIndirectCount(GetCurrentCommandBuffer()->GetVkCommandBuffer(),
        reinterpret_cast<VkBuffer>(indirect_draw_command_buffer->GetNativeBuffer()),
        offset,
        reinterpret_cast<VkBuffer>(count_buffer->GetNativeBuffer()),
        count_offset,
        max_draw_count,
        stride);
}

void VulkanCommandListContext::RHIDrawIndexedIndirectCount(RHIBuffer *indirect_draw_command_buffer, size_t offset,
    RHIBuffer *count_buffer, size_t count_offset, uint32_t max_draw_count, uint32_t stride)
{
    VulkanRHI::vkCmdDrawIndexedIndirectCount(GetCurrentCommandBuffer()->GetVkCommandBuffer(),
        reinterpret_cast<VkBuffer>(indirect_draw_command_buffer->GetNativeBuffer()),
        offset,
        reinterpret_cast<VkBuffer>(count_buffer->GetNativeBuffer()),
        count_offset,
        max_draw_count,
        stride);
}