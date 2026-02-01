#include "VulkanDescriptorHeap.h"

#include "Assertion.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"

static constexpr size_t GMaxSRVDescriptorCount = 1024 * 1024;
static constexpr size_t GMaxUAVDescriptorCount = 1024 * 1024;
static constexpr size_t GMaxSSVDescriptorCount = 1024;

static constexpr size_t GMaxPushConstantSize = 128;

static VkDescriptorSetLayoutCreateInfo GDescriptorSetLayoutInfo = {};

VulkanDescriptorRange::VulkanDescriptorRange(VulkanDevice& InDevice, const VulkanDescriptorRangeDesc& InDesc)
	: Device(InDevice)
	, Allocator(Desc.Capacity)
	, Desc(InDesc)
{
	Allocator.Init();
}

VulkanDescriptorRange::~VulkanDescriptorRange()
{
	VulkanRHI::vkDestroyDescriptorPool(Device.GetVkDevice(), DescriptorPool, nullptr);
	VulkanRHI::vkDestroyDescriptorSetLayout(Device.GetVkDevice(), DescriptorSetLayout, nullptr);

	Allocator.Reset();
}

void VulkanDescriptorRange::Allocate(RHIDescriptorResource& Resource)
{
	RHIDescriptorHandle Handle = Resource.GetDescriptorHandle();

	if (!Handle.IsValid())
	{
		Handle = Allocator.Allocate();

		Resource.SetResidentHandle(Handle, 0);
	}

	if (Resource.IsDirty())
	{
		//Update(Handle, Resource);

		Resource.SetResidentHandle(Handle, Resource.GetContentVersion());
	}
}

void VulkanDescriptorRange::Free(RHIDescriptorResource& Resource)
{
	RHIDescriptorHandle Handle = Resource.GetDescriptorHandle();
	if (!Handle.IsValid())
	{
		return;
	}

	Allocator.Free(Handle);
	Resource.ClearResidentHandle();
}

VulkanDescriptorHeap::VulkanDescriptorHeap()
{
}

VulkanDescriptorHeap::~VulkanDescriptorHeap()
{
}

void VulkanDescriptorHeap::Update(RHIDescriptorResource& Resource)
{
	VulkanDescriptorRange& Range = Ranges[Resource.GetDescriptorType()];

	Range.Allocate(Resource);
}