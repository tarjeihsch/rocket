#pragma once

#include <vector>
#include <queue>
#include <mutex>
#include <vulkan/vulkan_core.h>

class VulkanDevice;

// -----------------------------------------------------------------------------
// Core Types
// -----------------------------------------------------------------------------

struct BindlessHandle
{
	uint32_t Index = 0xFFFFFFFF;
	uint32_t Generation = 0;

	bool IsValid() const { return Index != 0xFFFFFFFF; }
};

// -----------------------------------------------------------------------------
// Component 1: Dynamic Ring Buffer
// Uses a "Linear Allocator" strategy for uploading frame parameters.
// -----------------------------------------------------------------------------
class VulkanDynamicRingBuffer
{
public:
	void Initialize(VulkanDevice& Device, uint32_t TotalSize);
	void Shutdown();

	void BeginFrame();

	// Allocates memory in the ring buffer and copies data to it.
	// Returns the Byte Offset within the GPU buffer.
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

// -----------------------------------------------------------------------------
// Component 2: Bindless Descriptor Heap
// Manages the unbounded arrays of Textures, Buffers, and Samplers.
// -----------------------------------------------------------------------------
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
	BindlessHandle RegisterTexture(VkImageView View, VkImageLayout Layout);
	BindlessHandle RegisterBuffer(VkBuffer Buffer, VkDeviceSize Range = VK_WHOLE_SIZE);
	BindlessHandle RegisterSampler(VkSampler Sampler);
	BindlessHandle RegisterStorageImage(VkImageView View, VkImageLayout Layout);

	// Resource Unregistration (Caller must ensure GPU is done using index!)
	void UnregisterTexture(BindlessHandle Handle);
	void UnregisterBuffer(BindlessHandle Handle);
	void UnregisterSampler(BindlessHandle Handle);
	void UnregisterStorageImage(BindlessHandle Handle);

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


// -----------------------------------------------------------------------------
// Component 3: The Manager
// Coordinates the Sets and Layouts.
// -----------------------------------------------------------------------------
class VulkanDescriptorManager
{
public:
	explicit VulkanDescriptorManager(VulkanDevice& Device);
	~VulkanDescriptorManager();

	// Accessors for sub-systems
	VulkanBindlessHeap& GetHeap() { return BindlessHeap; }
	VulkanDynamicRingBuffer& GetParameterBuffer() { return ParameterBuffer; }

	// The Global Pipeline Layout used by all Bindless Shaders
	VkPipelineLayout GetGlobalPipelineLayout() const { return GlobalPipelineLayout; }

	// Called at the start of a new frame to reset the ring buffer cursor
	void BeginFrame();

private:
	VulkanDevice& Device;
	VulkanBindlessHeap BindlessHeap;
	VulkanDynamicRingBuffer ParameterBuffer;
	VkPipelineLayout GlobalPipelineLayout = VK_NULL_HANDLE;
};