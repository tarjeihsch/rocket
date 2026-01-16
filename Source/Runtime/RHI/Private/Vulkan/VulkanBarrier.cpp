#include "VulkanBarrier.h"

#include "VulkanCommandBuffer.h"
#include "VulkanLoader.h"

void VulkanBarrier::AddTextureTransition(RHITexture* texture, const RHITextureAccessState& old_access_state, const RHITextureAccessState& new_access_state)
{
	VkImageMemoryBarrier2 image_barrier = {};
	image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	image_barrier.srcStageMask = VulkanRHI::GetVkPipelineStage(old_access_state.pipeline_stage);
	image_barrier.dstStageMask = VulkanRHI::GetVkPipelineStage(new_access_state.pipeline_stage);
	image_barrier.srcAccessMask = VulkanRHI::GetVkAccess(old_access_state.resource_access);
	image_barrier.dstAccessMask = VulkanRHI::GetVkAccess(new_access_state.resource_access);
	image_barrier.oldLayout = VulkanRHI::GetVulkanLayout(old_access_state.texture_layout);
	image_barrier.newLayout = VulkanRHI::GetVulkanLayout(new_access_state.texture_layout);
	image_barrier.srcQueueFamilyIndex = UINT32_MAX;
	image_barrier.dstQueueFamilyIndex = UINT32_MAX;
	image_barrier.image = static_cast<VkImage>(texture->GetNativeTexture());
	image_barrier.subresourceRange.baseMipLevel = 0;
	image_barrier.subresourceRange.levelCount = 1;
	image_barrier.subresourceRange.baseArrayLayer = 0;
	image_barrier.subresourceRange.layerCount = 1;
	image_barrier.subresourceRange.aspectMask = VulkanRHI::GetVkAspectFromFormat(texture->GetTextureDesc().format);

	image_barrier_data.push_back(image_barrier);
}

void VulkanBarrier::AddBufferTransition(RHIBuffer* buffer, const RHIBufferAccessState& old_access_state, const RHIBufferAccessState& new_access_state)
{
}

void VulkanBarrier::Execute(VulkanCommandBuffer* command_buffer)
{
	VkDependencyInfo DependencyInfo;
	DependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	DependencyInfo.pNext = nullptr;
	DependencyInfo.dependencyFlags = 0;
	DependencyInfo.memoryBarrierCount = memory_barrier_data.size();
	DependencyInfo.pMemoryBarriers = memory_barrier_data.data();
	DependencyInfo.bufferMemoryBarrierCount = buffer_barrier_data.size();
	DependencyInfo.pBufferMemoryBarriers = buffer_barrier_data.data();
	DependencyInfo.imageMemoryBarrierCount = image_barrier_data.size();
	DependencyInfo.pImageMemoryBarriers = image_barrier_data.data();
	
	VulkanRHI::vkCmdPipelineBarrier2KHR(command_buffer->GetVkCommandBuffer(), &DependencyInfo);
}