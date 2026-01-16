#pragma once

#include "Common/RHIBarrier.h"
#include "Common/RHITexture.h"

#include <vulkan/vulkan_core.h>

class VulkanCommandBuffer;
class RHICommandList;

class VulkanBarrier
{
public:
	void AddTextureTransition(RHITexture* texture, const RHITextureAccessState& old_access_state, const RHITextureAccessState& new_access_state);
	void AddBufferTransition(RHIBuffer* buffer, const RHIBufferAccessState& old_access_state, const RHIBufferAccessState& new_access_state);
	void Execute(VulkanCommandBuffer* command_buffer);

private:
	std::vector<VkImageMemoryBarrier2> image_barrier_data;
	std::vector<VkBufferMemoryBarrier2> buffer_barrier_data;
	std::vector<VkMemoryBarrier2> memory_barrier_data;
};
