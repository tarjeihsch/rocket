#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <array>

#include "Common/RHIDescriptorResource.h"
#include "Common/RHIDescriptorAllocator.h"

struct VulkanDevice;

struct VulkanDescriptorRangeDesc
{
	RHIDescriptorType Type;
	uint32_t Capacity;

	bool PartiallyBound = true;
	bool UpdateAfterBind = true;
	bool VariableCount = false;
};

class VulkanDescriptorRange
{
	VulkanDevice& Device;
	VulkanDescriptorRangeDesc Desc;

public:
	VulkanDescriptorRange(VulkanDevice& InDevice, const VulkanDescriptorRangeDesc& InDesc);
	~VulkanDescriptorRange();

	void Allocate(RHIDescriptorResource& Resource);
	void Free(RHIDescriptorResource& Resource);

protected:
	RHIDescriptorAllocator Allocator;

private:
	VkDescriptorSetLayout DescriptorSetLayout;
	VkDescriptorSet DescriptorSet;
	VkDescriptorPool DescriptorPool;
	VkDescriptorType DescriptorType;

	friend class VulkanDescriptorHeap;
};

class VulkanDescriptorHeap
{
public:
	VulkanDescriptorHeap();
	~VulkanDescriptorHeap();

	void Update(RHIDescriptorResource& Resource);

protected:
	std::vector<VulkanDescriptorRange> Ranges;

private:
	VkPipelineLayout PipelineLayout;
};