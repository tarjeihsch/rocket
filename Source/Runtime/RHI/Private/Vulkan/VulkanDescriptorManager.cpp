#include "VulkanDescriptorManager.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "Assertion.h"
#include <cstring> // for memcpy

// Helper to align memory offsets
template <typename T>
T Align(T Value, T Alignment)
{
	return (Value + Alignment - 1) & ~(Alignment - 1);
}

// =============================================================================
// VulkanDynamicRingBuffer Implementation
// =============================================================================

void VulkanDynamicRingBuffer::Initialize(VulkanDevice& InDevice, uint32_t TotalSize)
{
	Device = &InDevice;
	Capacity = TotalSize;
	FrameSize = TotalSize / MaxFramesInFlight;
	// 1. Create the Buffer (Storage Buffer Usage)
	VkBufferCreateInfo BufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	BufferInfo.size = TotalSize;
	BufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VulkanRHI::vkCreateBuffer(Device->GetVkDevice(), &BufferInfo, nullptr, &Buffer);

	// 2. Allocate Memory (Host Visible + Coherent usually, or Device Local + Host Visible for ReBAR)
	VkMemoryRequirements MemReqs;
	VulkanRHI::vkGetBufferMemoryRequirements(Device->GetVkDevice(), Buffer, &MemReqs);

	VkMemoryAllocateInfo AllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	AllocInfo.allocationSize = MemReqs.size;
	// Simplification: finding the right memory type index
	AllocInfo.memoryTypeIndex = Device->FindMemoryType(MemReqs.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VulkanRHI::vkAllocateMemory(Device->GetVkDevice(), &AllocInfo, nullptr, &Memory);
	VulkanRHI::vkBindBufferMemory(Device->GetVkDevice(), Buffer, Memory, 0);

	// 3. Map Persistent
	VulkanRHI::vkMapMemory(Device->GetVkDevice(), Memory, 0, TotalSize, 0, &MappedData);

	// 4. Create Descriptor Set for this single buffer
	// This allows us to bind it as a Storage Buffer at Set 1, Binding 0
	VkDescriptorSetLayoutBinding Binding = {};
	Binding.binding = 0;
	Binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	Binding.descriptorCount = 1;
	Binding.stageFlags = VK_SHADER_STAGE_ALL;

	VkDescriptorSetLayoutCreateInfo LayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	LayoutInfo.bindingCount = 1;
	LayoutInfo.pBindings = &Binding;
	VulkanRHI::vkCreateDescriptorSetLayout(Device->GetVkDevice(), &LayoutInfo, nullptr, &DescriptorSetLayout);

	// Pool
	VkDescriptorPoolSize PoolSize = { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 };
	VkDescriptorPoolCreateInfo PoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	PoolInfo.maxSets = 1;
	PoolInfo.poolSizeCount = 1;
	PoolInfo.pPoolSizes = &PoolSize;
	VulkanRHI::vkCreateDescriptorPool(Device->GetVkDevice(), &PoolInfo, nullptr, &DescriptorPool);

	// Allocate Set
	VkDescriptorSetAllocateInfo SetAllocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	SetAllocInfo.descriptorPool = DescriptorPool;
	SetAllocInfo.descriptorSetCount = 1;
	SetAllocInfo.pSetLayouts = &DescriptorSetLayout;
	VulkanRHI::vkAllocateDescriptorSets(Device->GetVkDevice(), &SetAllocInfo, &DescriptorSet);

	// Write Descriptor Immediately (It points to the whole buffer)
	VkDescriptorBufferInfo BufferInfoDesc = {};
	BufferInfoDesc.buffer = Buffer;
	BufferInfoDesc.offset = 0;
	BufferInfoDesc.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	Write.dstSet = DescriptorSet;
	Write.dstBinding = 0;
	Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	Write.descriptorCount = 1;
	Write.pBufferInfo = &BufferInfoDesc;
	VulkanRHI::vkUpdateDescriptorSets(Device->GetVkDevice(), 1, &Write, 0, nullptr);
}

void VulkanDynamicRingBuffer::Shutdown()
{
	if (DescriptorSetLayout) VulkanRHI::vkDestroyDescriptorSetLayout(Device->GetVkDevice(), DescriptorSetLayout, nullptr);
	if (DescriptorPool) VulkanRHI::vkDestroyDescriptorPool(Device->GetVkDevice(), DescriptorPool, nullptr);
	if (Buffer) VulkanRHI::vkDestroyBuffer(Device->GetVkDevice(), Buffer, nullptr);
	if (Memory) VulkanRHI::vkFreeMemory(Device->GetVkDevice(), Memory, nullptr);
}

void VulkanDynamicRingBuffer::BeginFrame()
{
	uint32_t FrameIndex = CurrentFrameIndex % MaxFramesInFlight;
	CurrentOffset = FrameIndex * FrameSize;
	CurrentFrameIndex++;
}

uint32_t VulkanDynamicRingBuffer::Allocate(const void* Data, uint32_t Size)
{
	// Ensure 16-byte alignment for the struct pulls
	const uint32_t Alignment = 16;
	uint32_t AlignedOffset = Align(CurrentOffset, Alignment);

	// Ensure we don't leak into the next frame's partition
	uint32_t FrameIndex = (CurrentFrameIndex - 1) % MaxFramesInFlight;
	uint32_t FrameEnd = (FrameIndex + 1) * FrameSize;
	ensure(AlignedOffset + Size <= FrameEnd);

	memcpy(static_cast<uint8_t*>(MappedData) + AlignedOffset, Data, Size);
	CurrentOffset = AlignedOffset + Size;

	return AlignedOffset;
}
// =============================================================================
// VulkanBindlessHeap Implementation
// =============================================================================

void VulkanBindlessHeap::ResourcePool::Init(uint32_t Count, uint32_t Binding)
{
	MaxCapacity = Count;
	BindingIndex = Binding;
	Generations.resize(Count, 0);

	// Fill free list
	for (uint32_t i = 0; i < Count; ++i)
	{
		FreeIndices.push(i);
	}
}

uint32_t VulkanBindlessHeap::ResourcePool::Allocate()
{
	ensure(!FreeIndices.empty());
	uint32_t Index = FreeIndices.front();
	FreeIndices.pop();
	return Index;
}

void VulkanBindlessHeap::ResourcePool::Free(uint32_t Index)
{
	Generations[Index]++;
	FreeIndices.push(Index);
}

void VulkanBindlessHeap::Initialize(VulkanDevice& InDevice, const Config& Cfg)
{
	Device = &InDevice;

	TexturePool.Init(Cfg.MaxTextures, 0); // Binding 0
	BufferPool.Init(Cfg.MaxBuffers, 1);   // Binding 1
	SamplerPool.Init(Cfg.MaxSamplers, 2); // Binding 2
	StorageImagePool.Init(Cfg.MaxStorageImages, 3); // Binding 3

	VkDescriptorPoolSize Sizes[] = {
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Cfg.MaxTextures },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Cfg.MaxBuffers },
		{ VK_DESCRIPTOR_TYPE_SAMPLER, Cfg.MaxSamplers },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Cfg.MaxStorageImages } // NEW
	};

	VkDescriptorPoolCreateInfo PoolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	PoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	PoolInfo.maxSets = 1;
	PoolInfo.poolSizeCount = 4;
	PoolInfo.pPoolSizes = Sizes;

	VulkanRHI::vkCreateDescriptorPool(Device->GetVkDevice(), &PoolInfo, nullptr, &DescriptorPool);

	VkDescriptorBindingFlags Flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	VkDescriptorBindingFlags BindingFlags[] = { Flags, Flags, Flags, Flags };

	VkDescriptorSetLayoutBinding Bindings[] = {
		{ 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Cfg.MaxTextures, VK_SHADER_STAGE_ALL, nullptr },
		{ 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Cfg.MaxBuffers, VK_SHADER_STAGE_ALL, nullptr },
		{ 2, VK_DESCRIPTOR_TYPE_SAMPLER, Cfg.MaxSamplers, VK_SHADER_STAGE_ALL, nullptr },
		{ 3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Cfg.MaxStorageImages, VK_SHADER_STAGE_ALL, nullptr },
	};

	VkDescriptorSetLayoutBindingFlagsCreateInfo FlagsInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
	FlagsInfo.bindingCount = 4;
	FlagsInfo.pBindingFlags = BindingFlags;

	VkDescriptorSetLayoutCreateInfo LayoutInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	LayoutInfo.pNext = &FlagsInfo;
	LayoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	LayoutInfo.bindingCount = 4;
	LayoutInfo.pBindings = Bindings;

	VulkanRHI::vkCreateDescriptorSetLayout(Device->GetVkDevice(), &LayoutInfo, nullptr, &DescriptorSetLayout);

	VkDescriptorSetAllocateInfo AllocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	AllocInfo.descriptorPool = DescriptorPool;
	AllocInfo.descriptorSetCount = 1;
	AllocInfo.pSetLayouts = &DescriptorSetLayout;

	VulkanRHI::vkAllocateDescriptorSets(Device->GetVkDevice(), &AllocInfo, &DescriptorSet);
}

void VulkanBindlessHeap::Shutdown()
{
	if (DescriptorSetLayout) VulkanRHI::vkDestroyDescriptorSetLayout(Device->GetVkDevice(), DescriptorSetLayout, nullptr);
	if (DescriptorPool) VulkanRHI::vkDestroyDescriptorPool(Device->GetVkDevice(), DescriptorPool, nullptr);
}

RHIDescriptorHandle VulkanBindlessHeap::RegisterTexture(VkImageView View, VkImageLayout Layout)
{
	uint32_t Index = TexturePool.Allocate();

	VkDescriptorImageInfo Info = {};
	Info.imageView = View;
	Info.imageLayout = Layout;
	Info.sampler = VK_NULL_HANDLE;

	VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	Write.dstSet = DescriptorSet;
	Write.dstBinding = TexturePool.BindingIndex;
	Write.dstArrayElement = Index;
	Write.descriptorCount = 1;
	Write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	Write.pImageInfo = &Info;

	VulkanRHI::vkUpdateDescriptorSets(Device->GetVkDevice(), 1, &Write, 0, nullptr);

	return { Index, TexturePool.Generations[Index] };
}

RHIDescriptorHandle VulkanBindlessHeap::RegisterBuffer(VkBuffer Buffer, VkDeviceSize Range)
{
	uint32_t Index = BufferPool.Allocate();

	VkDescriptorBufferInfo Info = {};
	Info.buffer = Buffer;
	Info.offset = 0;
	Info.range = Range;

	VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	Write.dstSet = DescriptorSet;
	Write.dstBinding = BufferPool.BindingIndex;
	Write.dstArrayElement = Index;
	Write.descriptorCount = 1;
	Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	Write.pBufferInfo = &Info;

	VulkanRHI::vkUpdateDescriptorSets(Device->GetVkDevice(), 1, &Write, 0, nullptr);

	return { Index, BufferPool.Generations[Index] };
}

RHIDescriptorHandle VulkanBindlessHeap::RegisterSampler(VkSampler Sampler)
{
	// 1. Allocate an index from the Sampler Pool
	uint32_t Index = SamplerPool.Allocate();

	// 2. Prepare the Descriptor Info
	VkDescriptorImageInfo Info = {};
	Info.sampler = Sampler;
	// imageView and imageLayout are ignored for pure Sampler descriptors
	Info.imageView = VK_NULL_HANDLE;
	Info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// 3. Update the Global Descriptor Set
	// Note: Use VK_DESCRIPTOR_TYPE_SAMPLER
	VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	Write.dstSet = DescriptorSet;
	Write.dstBinding = SamplerPool.BindingIndex; // Should be 2 based on previous config
	Write.dstArrayElement = Index;
	Write.descriptorCount = 1;
	Write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	Write.pImageInfo = &Info;

	VulkanRHI::vkUpdateDescriptorSets(Device->GetVkDevice(), 1, &Write, 0, nullptr);

	// 4. Return the Handle
	return { Index, SamplerPool.Generations[Index] };
}

RHIDescriptorHandle VulkanBindlessHeap::RegisterStorageImage(VkImageView View, VkImageLayout Layout)
{
	uint32_t Index = StorageImagePool.Allocate();

	VkDescriptorImageInfo Info = {};
	Info.imageView = View;
	Info.imageLayout = Layout;
	// Storage images don't use samplers

	VkWriteDescriptorSet Write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	Write.dstSet = DescriptorSet;
	Write.dstBinding = StorageImagePool.BindingIndex; // 3
	Write.dstArrayElement = Index;
	Write.descriptorCount = 1;
	Write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	Write.pImageInfo = &Info;

	VulkanRHI::vkUpdateDescriptorSets(Device->GetVkDevice(), 1, &Write, 0, nullptr);

	return { Index, StorageImagePool.Generations[Index] };
}

void VulkanBindlessHeap::UnregisterSampler(RHIDescriptorHandle Handle)
{
	// Validate Handle validity and Generation to prevent stale deletion
	if (Handle.IsValid() && SamplerPool.Generations[Handle.Index] == Handle.Version)
	{
		SamplerPool.Free(Handle.Index);
	}
}

void VulkanBindlessHeap::UnregisterStorageImage(RHIDescriptorHandle Handle)
{
	if (Handle.IsValid() && StorageImagePool.Generations[Handle.Index] == Handle.Version)
	{
		StorageImagePool.Free(Handle.Index);
	}
}

void VulkanBindlessHeap::UnregisterTexture(RHIDescriptorHandle Handle)
{
	if (Handle.IsValid() && TexturePool.Generations[Handle.Index] == Handle.Version)
	{
		TexturePool.Free(Handle.Index);
	}
}

void VulkanBindlessHeap::UnregisterBuffer(RHIDescriptorHandle Handle)
{
	if (Handle.IsValid() && BufferPool.Generations[Handle.Index] == Handle.Version)
	{
		BufferPool.Free(Handle.Index);
	}
}

// =============================================================================
// VulkanDescriptorManager Implementation
// =============================================================================

VulkanDescriptorManager::VulkanDescriptorManager(VulkanDevice& InDevice)
	: Device(InDevice)
{
	// 1. Initialize Subsystems
	VulkanBindlessHeap::Config HeapConfig;
	HeapConfig.MaxTextures = 16384;
	HeapConfig.MaxBuffers = 4096;
	BindlessHeap.Initialize(Device, HeapConfig);

	// 64 KB Parameter Buffer (Should be enough for any reasonable frame)
	ParameterBuffer.Initialize(Device, 64 * 1024);

	// 2. Create Global Pipeline Layout
	// This layout is shared by ALL pipelines in the engine.
	// Set 0: Bindless Heap
	// Set 1: Parameter Buffer

	VkDescriptorSetLayout SetLayouts[] = {
		BindlessHeap.GetLayout(),
		ParameterBuffer.GetLayout()
	};

	// Define Push Constant (Just the offset into the parameter buffer)
	VkPushConstantRange PCRange = {};
	PCRange.stageFlags = VK_SHADER_STAGE_ALL;
	PCRange.offset = 0;
	PCRange.size = sizeof(uint32_t); // We only push 4 bytes!

	VkPipelineLayoutCreateInfo PipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	PipelineLayoutInfo.setLayoutCount = 2;
	PipelineLayoutInfo.pSetLayouts = SetLayouts;
	PipelineLayoutInfo.pushConstantRangeCount = 1;
	PipelineLayoutInfo.pPushConstantRanges = &PCRange;

	VulkanRHI::vkCreatePipelineLayout(Device.GetVkDevice(), &PipelineLayoutInfo, nullptr, &GlobalPipelineLayout);
}

VulkanDescriptorManager::~VulkanDescriptorManager()
{
	if (GlobalPipelineLayout) VulkanRHI::vkDestroyPipelineLayout(Device.GetVkDevice(), GlobalPipelineLayout, nullptr);
	ParameterBuffer.Shutdown();
	BindlessHeap.Shutdown();
}

void VulkanDescriptorManager::BeginFrame()
{
	ParameterBuffer.BeginFrame();
}