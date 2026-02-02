#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <vulkan/vulkan_core.h>
#include <Common/RHIDescriptorResource.h>

class VulkanDevice;

class VulkanDynamicRingBuffer
{
public:
	void Initialize(VulkanDevice& Device, uint32_t TotalSize);
	void Shutdown();

	void BeginFrame();

	uint32_t Allocate(const void* Data, uint32_t Size);

	VkDescriptorSet GetDescriptorSet() const { return DescriptorSet; }
	VkDescriptorSetLayout GetLayout() const { return DescriptorSetLayout; }

private:
	VulkanDevice* Device = nullptr;
	VkBuffer Buffer = VK_NULL_HANDLE;
	VkDeviceMemory Memory = VK_NULL_HANDLE;
	void* MappedData = nullptr;

	uint32_t Capacity = 0;
	uint32_t CurrentOffset = 0;
	uint32_t FrameSize = 0;
	uint32_t CurrentFrameIndex = 0;
	static constexpr uint32_t MaxFramesInFlight = 3;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;
};

class VulkanBindlessHeap
{
public:
	struct Config
	{
		uint32_t MaxTextures = 16384;
		uint32_t MaxBuffers = 4096;
		uint32_t MaxSamplers = 1024;
		uint32_t MaxStorageImages = 4096;
	};

	void Initialize(VulkanDevice& Device, const Config& Config);
	void Shutdown();

	// Resource Registration
	RHIDescriptorHandle RegisterTexture(VkImageView View, VkImageLayout Layout);
	RHIDescriptorHandle RegisterBuffer(VkBuffer Buffer, VkDeviceSize Range = VK_WHOLE_SIZE);
	RHIDescriptorHandle RegisterSampler(VkSampler Sampler);
	RHIDescriptorHandle RegisterStorageImage(VkImageView View, VkImageLayout Layout);

	// Resource Unregistration (Caller must ensure GPU is done using index!)
	void UnregisterTexture(RHIDescriptorHandle Handle);
	void UnregisterBuffer(RHIDescriptorHandle Handle);
	void UnregisterSampler(RHIDescriptorHandle Handle);
	void UnregisterStorageImage(RHIDescriptorHandle Handle);

	VkDescriptorSet GetDescriptorSet() const { return DescriptorSet; }
	VkDescriptorSetLayout GetLayout() const { return DescriptorSetLayout; }

private:
	// Internal Helper for Free List Management
	struct ResourcePool
	{
		std::queue<uint32_t> FreeIndices;
		std::vector<uint32_t> Generations;
		uint32_t MaxCapacity = 0;
		uint32_t BindingIndex = 0;

		void Init(uint32_t Count, uint32_t Binding);
		uint32_t Allocate();
		void Free(uint32_t Index);
	};

	VulkanDevice* Device = nullptr;
	VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
	VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorSet DescriptorSet = VK_NULL_HANDLE;

	ResourcePool TexturePool; // Binding 0
	ResourcePool BufferPool;  // Binding 1
	ResourcePool SamplerPool; // Binding 2
	ResourcePool StorageImagePool; // Binding 3
};

class VulkanDescriptorManager
{
public:
	explicit VulkanDescriptorManager(VulkanDevice& Device);
	~VulkanDescriptorManager();

	VulkanBindlessHeap& GetHeap() { return BindlessHeap; }
	VulkanDynamicRingBuffer& GetParameterBuffer() { return ParameterBuffer; }

	VkPipelineLayout GetGlobalPipelineLayout() const { return GlobalPipelineLayout; }

	void BeginFrame();

private:
	VulkanDevice& Device;
	VulkanBindlessHeap BindlessHeap;
	VulkanDynamicRingBuffer ParameterBuffer;
	VkPipelineLayout GlobalPipelineLayout;
};