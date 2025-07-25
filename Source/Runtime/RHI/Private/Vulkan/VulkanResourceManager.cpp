#include "VulkanResourceManager.h"

#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "Assert.h"
#include  <cstring>
#include <Vulkan/VulkanRHIMinimal.h>

void CreatePoolAllocator(FVulkanDevice& p_Device, VulkanPoolAllocator& OutPoolAllocator, PoolAllocatorType Type, VkDeviceSize Size)
{
    VkResult Result = VK_SUCCESS;

    VkPhysicalDeviceMemoryProperties MemoryProperties;
    VulkanRHI::vkGetPhysicalDeviceMemoryProperties(p_Device.GetVkPhysicalDevice(), &MemoryProperties);

    // All memory types are acceptable but regarded unsafe for production. Use vkGetImageMemoryRequirements or vkGetBufferMemoryRequirements instead.
    // TODO: Query correct memory type for proper performance. Different hardware might differ in TypeIndex order.
    uint32_t MemoryTypeBits = 0xFFFFFFFF;
    uint32_t TypeIndex = UINT32_MAX;

    for (uint32_t Index = 0; Index < MemoryProperties.memoryTypeCount; ++Index)
    {
        if ((MemoryTypeBits & (1 << Index)) && (MemoryProperties.memoryTypes[Index].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
        {
            TypeIndex = Index;
            break;
        }
    }

    RK_ASSERT(TypeIndex != UINT32_MAX);

    VkMemoryAllocateInfo AllocInfo = {};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = Size;
    AllocInfo.memoryTypeIndex = TypeIndex;

    Result = VulkanRHI::vkAllocateMemory(p_Device.GetVkDevice(), &AllocInfo, nullptr, &OutPoolAllocator.PhysicalMemory);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate memory.");

    VmaVirtualBlockCreateInfo BlockCreateInfo = {};
    BlockCreateInfo.size = VK_WHOLE_SIZE;
    BlockCreateInfo.flags = 0;

    Result = vmaCreateVirtualBlock(&BlockCreateInfo, &OutPoolAllocator.VirtualMemory);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create virtual block.");
}

void DestroyPoolAllocator(VulkanPoolAllocator& PoolAllocator)
{
    VulkanRHI::vkFreeMemory(GetVulkanRHIMinimal()->RHIGetVkDevice(), PoolAllocator.PhysicalMemory, nullptr);
    vmaClearVirtualBlock(PoolAllocator.VirtualMemory);
    vmaDestroyVirtualBlock(PoolAllocator.VirtualMemory);
}

void ResetPoolAllocator(VulkanPoolAllocator &PoolAllocator)
{
    vmaClearVirtualBlock(PoolAllocator.VirtualMemory);
    PoolAllocator.Offset = 0;
}


VulkanResourceManager::VulkanResourceManager(FVulkanDevice &p_Device, uint32_t p_FramesInFlight)
    : m_Device(p_Device), m_FramesInFlight(p_FramesInFlight)
{
    Init();
}

VulkanResourceManager::~VulkanResourceManager()
{
    Shutdown();
}

void VulkanResourceManager::Shutdown()
{
    GC();

    for (auto& Record : m_List_Buffer)
    {
        if (Record.Persistent)
        {
            vmaDestroyBuffer(m_Device.GetVmaAllocator(), Record.m_Buffer, Record.Allocation);
        }
    }

    for (auto& Record : m_List_Image)
    {
        if (Record.Persistent)
        {
            //vmaDestroyImage(m_Device.GetVmaAllocator(), Record.m_Image, Record.Allocation);
            //VulkanRHI::vkDestroyImageView(GetVulkanRHIMinimal()->RHIGetVkDevice(), Record.m_View, nullptr);
        }
    }

    DestroyPoolAllocator(*ImagePoolAllocator);
    DestroyPoolAllocator(*BufferPoolAllocator);
}

void VulkanResourceManager::Init()
{
    BufferPoolAllocator = new VulkanPoolAllocator();
    ImagePoolAllocator = new VulkanPoolAllocator();

    CreatePoolAllocator(m_Device, *BufferPoolAllocator, PoolAllocatorType::Buffer, 32 * 1024 * 1024);
    CreatePoolAllocator(m_Device, *ImagePoolAllocator, PoolAllocatorType::Image, 32 * 1024 * 1024);
}

BufferHandle VulkanResourceManager::AddBuffer(BufferDesc &p_Desc)
{
    BufferHandle Handle = AllocHandle_Buffer();
    auto& Record = m_List_Buffer[Handle.Index];
    Record.Persistent = p_Desc.Persistent;

    VkBufferCreateInfo BufferCreateInfo = {};
    BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInfo.size = p_Desc.Size;
    BufferCreateInfo.usage = p_Desc.UsageFlags;
    BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (p_Desc.Persistent)
    {
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = p_Desc.MemoryUsage;
        AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VkResult Result = vmaCreateBuffer(m_Device.GetVmaAllocator(), &BufferCreateInfo, &AllocationCreateInfo, &Record.m_Buffer, &Record.Allocation, nullptr);
        RK_ASSERT(Result == VK_SUCCESS);
    }
    else
    {
        // Immediately defer transient resource pending free
        DestroyBuffer(Handle);
    }

    return Handle;
}

void VulkanResourceManager::DestroyBuffer(BufferHandle &Handle)
{
    PendingFreeDesc Desc = {};
    Desc.Handle = Handle;
    m_PendingQueue.push_back(Desc);
}

BufferHandle VulkanResourceManager::AllocHandle_Buffer()
{
    BufferHandle Handle;
    Handle.Type = ResourceType::Buffer;

    uint32_t Index = ~0u;

    if (m_FreeList_Buffer.empty())
    {
        Index = static_cast<uint32_t>(m_List_Buffer.size());
        BufferRecord Record = {};
        Record.Generation = 1;
        Record.Allocation = VK_NULL_HANDLE;
        Record.m_Buffer   = VK_NULL_HANDLE;
        m_List_Buffer.push_back(Record);
        Handle.Generation = 1;
    }
    else
    {
        Index = m_FreeList_Buffer.back();
        m_FreeList_Buffer.pop_back();
        auto& Record = m_List_Buffer[Index];
        Record.Generation += 1;
        Handle.Generation = Record.Generation;
    }

    Handle.Index = Index;
    return Handle;
}

ImageHandle VulkanResourceManager::AllocHandle_Image()
{
    ImageHandle  Handle;
    Handle.Type = ResourceType::Image;

    uint32_t Index = ~0u;

    if (m_FreeList_Image.empty())
    {
        Index = static_cast<uint32_t>(m_FreeList_Image.size());
        ImageRecord Record = {};
        Record.Generation = 1;
        Record.Allocation = VK_NULL_HANDLE;
        Record.m_Image   = VK_NULL_HANDLE;
        m_List_Image.push_back(Record);
        Handle.Generation = 1;
    }
    else
    {
        Index = m_FreeList_Image.back();
        m_FreeList_Image.pop_back();
        auto& Record = m_List_Image[Index];
        Record.Generation += 1;
        Handle.Generation = Record.Generation;
    }

    Handle.Index = Index;
    return Handle;
}

VkBuffer VulkanResourceManager::GetBuffer(BufferHandle p_Handle)
{
    auto& Resource = m_List_Buffer[p_Handle.Index];
    RK_ASSERT(Resource.Generation == p_Handle.Generation);
    return Resource.m_Buffer;
}

void VulkanResourceManager::GC()
{
    while (!m_PendingQueue.empty())
    {
        auto Front = m_PendingQueue.front();
        m_PendingQueue.pop_front();

        uint32_t Index = Front.Handle.Index;

        if (Front.Handle.Type == ResourceType::Buffer)
        {
            auto& Record = m_List_Buffer[Index];
            vmaDestroyBuffer(m_Device.GetVmaAllocator(), Record.m_Buffer, Record.Allocation);
            Record.m_Buffer = VK_NULL_HANDLE;
            Record.Allocation = nullptr;
            m_FreeList_Buffer.push_back(Index);
        }
        else
        {
            auto& Record = m_List_Image[Index];
            vmaDestroyImage(m_Device.GetVmaAllocator(), Record.m_Image, Record.Allocation);
            VulkanRHI::vkDestroyImageView(GetVulkanRHIMinimal()->RHIGetVkDevice(), Record.m_View, nullptr);
            Record.m_Image = VK_NULL_HANDLE;
            Record.Allocation = nullptr;
            m_FreeList_Image.push_back(Index);
        }
    }

    ResetPoolAllocator(*ImagePoolAllocator);
    ResetPoolAllocator(*BufferPoolAllocator);
}

void VulkanResourceManager::MapBuffer(BufferHandle p_Handle, void* p_Data, size_t p_Size, size_t p_Offset)
{
    const BufferRecord& Record = m_List_Buffer[p_Handle.Index];

    uint8_t* Base = nullptr;
    vmaMapMemory(m_Device.GetVmaAllocator(), Record.Allocation, reinterpret_cast<void**>(&Base));
    memcpy(Base + p_Offset, p_Data, p_Size);
    vmaUnmapMemory(m_Device.GetVmaAllocator(), Record.Allocation);
}

ImageHandle VulkanResourceManager::AddImage(ImageDesc &p_Desc)
{
    ImageHandle Handle = AllocHandle_Image();
    auto& Record = m_List_Image[Handle.Index];
    Record.Persistent = p_Desc.Persistent;

    VkImageCreateInfo ImageCreateInfo = {};
    ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInfo.format = p_Desc.Format;
    ImageCreateInfo.extent = p_Desc.Extent;
    ImageCreateInfo.mipLevels = p_Desc.MipLevels;
    ImageCreateInfo.arrayLayers = p_Desc.Layers;
    ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInfo.usage = p_Desc.UsageFlags;
    ImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (p_Desc.Persistent)
    {
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = p_Desc.MemoryUsage;
        AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VkResult Result = VulkanRHI::vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &ImageCreateInfo, nullptr, &Record.m_Image);
        RK_ASSERT(Result == VK_SUCCESS);
    }
    else
    {
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = p_Desc.MemoryUsage;
        AllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        VkResult Result = VulkanRHI::vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &ImageCreateInfo, nullptr, &Record.m_Image);
        RK_ASSERT(Result == VK_SUCCESS);

        VkMemoryRequirements MemoryRequirements;
        VulkanRHI::vkGetImageMemoryRequirements(GetVulkanRHIMinimal()->RHIGetVkDevice(), Record.m_Image, &MemoryRequirements);

        VmaVirtualAllocationCreateInfo AllocInfo = {};
        AllocInfo.size = MemoryRequirements.size;
        AllocInfo.alignment = MemoryRequirements.alignment;
        VmaVirtualAllocation Allocation;
        VkDeviceSize Offset;

        Result = vmaVirtualAllocate(ImagePoolAllocator->VirtualMemory, &AllocInfo, &Allocation, &Offset);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to allocate transient image memory.");

        VulkanRHI::vkBindImageMemory(GetVulkanRHIMinimal()->RHIGetVkDevice(), Record.m_Image, ImagePoolAllocator->PhysicalMemory, Offset);

        VkImageViewCreateInfo ImageViewCreateInfo = {};
        ImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        ImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        ImageViewCreateInfo.image = Record.m_Image;
        ImageViewCreateInfo.format = p_Desc.Format;
        ImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        ImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        ImageViewCreateInfo.subresourceRange.levelCount = 1;
        ImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        ImageViewCreateInfo.subresourceRange.layerCount = 1;
        ImageViewCreateInfo.subresourceRange.aspectMask = p_Desc.AspectFlags;

        Result = VulkanRHI::vkCreateImageView(GetVulkanRHIMinimal()->RHIGetVkDevice(), &ImageViewCreateInfo, nullptr, &Record.m_View);
        RK_ASSERT(Result == VK_SUCCESS, "Failed to create image view.");

        // Immediately defer transient resource pending free
        DestroyBuffer(Handle);
    }

    return Handle;
}

VkImage VulkanResourceManager::GetImage(BufferHandle p_Handle)
{
    auto& Resource = m_List_Image[p_Handle.Index];
    RK_ASSERT(Resource.Generation == p_Handle.Generation);
    return Resource.m_Image;
}