#pragma once

#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include <vector>
#include <deque>

struct VulkanPoolAllocator
{
    VmaVirtualBlock VirtualMemory = nullptr;
    VkDeviceMemory PhysicalMemory = nullptr;
    VkDeviceSize Offset = 0;
};

enum class PoolAllocatorType { Image, Buffer };

enum class ResourceType : uint32_t { None = 0, Image, Buffer };

struct ResourceHandle
{
    uint32_t Index = ~0u;
    uint32_t Generation = 0;
    ResourceType Type = ResourceType::None;

    inline bool IsValid() const
    {
        return Index != ~0u;
    }
};

using BufferHandle = ResourceHandle;
using ImageHandle = ResourceHandle;

struct BufferDesc
{
    VkDeviceSize Size;
    VkBufferUsageFlags UsageFlags;
    VmaMemoryUsage MemoryUsage;
    const char* DebugName;
    bool Persistent;
};

struct ImageDesc
{
    VkExtent3D Extent;
    VkFormat Format;
    uint32_t MipLevels;
    uint32_t Layers = 1;
    VkSampleCountFlagBits SampleCount;
    VkImageUsageFlags UsageFlags;
    VmaMemoryUsage MemoryUsage;
    VkImageAspectFlags AspectFlags;
    const char* DebugName;
    bool Persistent;
};

class FVulkanDevice;

// As we are mixing both transient and persistent resource objects in one large vector, true O(1) invalidation reset on transient
// objects is not possible. Transient objects will be reset at speed of O(N) where N = number of elements in deque.
// Might be changed in the future -- where the alternative is to separate transient and persistent resources into their own vectors.
// This will guarantee O(1) invalidation of any transient resources, as we can just clear the vector instead of walking the deque.

class VulkanResourceManager
{
public:
    VulkanResourceManager(FVulkanDevice& p_Device, uint32_t p_FramesInFlight = 2);
    ~VulkanResourceManager();

    BufferHandle AddBuffer(BufferDesc& Desc);
    void MarkPendingFree(BufferHandle& p_Handle);
    VkBuffer GetBuffer(BufferHandle p_Handle);
    void MapBuffer(BufferHandle p_Handle, void* p_Data, size_t p_Size, size_t p_Offset);

    ImageHandle AddImage(ImageDesc& p_Desc);
    VkImage GetImage(BufferHandle p_Handle);

    void GC();

private:
    void Init();
    void Shutdown();

private:
    BufferHandle AllocHandle_Buffer();
    ImageHandle AllocHandle_Image();

private:
    struct Record
    {
        VmaAllocation Allocation;
        uint32_t Generation;
        bool Persistent;
    };

    struct BufferRecord : Record
    {
        VkBuffer m_Buffer;
    };

    struct ImageRecord : Record
    {
        VkImage m_Image;
        VkImageView m_View;
    };

    struct PendingFreeDesc
    {
        //uint64_t Fence;
        BufferHandle Handle;
    };

private:
    FVulkanDevice& m_Device;
    uint32_t m_FramesInFlight;

    std::vector<BufferRecord> m_List_Buffer;
    std::vector<ImageRecord> m_List_Image;

    std::vector<uint32_t>  m_FreeList_Buffer;
    std::vector<uint32_t>  m_FreeList_Image;

    std::deque<PendingFreeDesc> m_PendingQueue;

    VulkanPoolAllocator* ImagePoolAllocator;
    VulkanPoolAllocator* BufferPoolAllocator;
};