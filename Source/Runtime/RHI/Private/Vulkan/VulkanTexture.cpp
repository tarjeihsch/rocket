#include "VulkanTexture.h"

#include "Assert.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanResourceManager.h"
#include "Vulkan/VulkanRHIMinimal.h"

#include <cassert>

FVulkanTexture::FVulkanTexture(FVulkanDevice &InDevice, FRHITextureDesc &InDesc)
    : Device(InDevice), Desc(InDesc), Allocation(nullptr)
{
    Alloc();
}

FVulkanTexture::FVulkanTexture(FVulkanDevice &InDevice, FRHITextureDesc &InDesc, VkImage InImage)
    : Device(InDevice), Desc(InDesc), Image(InImage), Allocation(nullptr)
{
}

FVulkanTexture::~FVulkanTexture()
{
    Free();
}

const FRHITextureDesc& FVulkanTexture::RHIGetTextureDesc() const
{
    return Desc;
}

void FVulkanTexture::Alloc()
{
    VkImageCreateInfo CreateInfo = VulkanRHI::GetVkImageCreateInfo(Desc.Extent, Desc.Format, Desc.Usage);

    VkResult Result = VulkanRHI::vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CreateInfo, nullptr, &Image);
    RK_ASSERT(Result == VK_SUCCESS);

    if ((Desc.Flags & ERHITextureFlags::Unmanaged) == ERHITextureFlags::Unmanaged)
    {
        VkMemoryRequirements MemoryRequirements;
        VulkanRHI::vkGetImageMemoryRequirements(GetVulkanRHIMinimal()->RHIGetVkDevice(), Image, &MemoryRequirements);

        VmaVirtualAllocationCreateInfo AllocInfo = {};
        AllocInfo.size = MemoryRequirements.size;
        AllocInfo.alignment = MemoryRequirements.alignment;

        VmaVirtualAllocation Allocation;
        VkDeviceSize Offset;
        Result = vmaVirtualAllocate(Desc.VirtualMemory, &AllocInfo, &Allocation, &Offset);
    }
    else
    {
        // We own the allocation, allocate a persistent image
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        Result = vmaCreateImage(Device.GetVmaAllocator(), &CreateInfo, &AllocationCreateInfo, &Image, &Allocation, nullptr);
        RK_ASSERT(Result == VK_SUCCESS);
    }
}

void FVulkanTexture::Free()
{
    if ((Desc.Flags & ERHITextureFlags::Unmanaged) == ERHITextureFlags::Unmanaged)
    {
        // Only destroy handle, memory will be freed externally
        VulkanRHI::vkDestroyImage(Device.GetVkDevice(), Image, nullptr);
    }
    else
    {
        // We own the allocation, free memory
        vmaDestroyImage(Device.GetVmaAllocator(), Image, Allocation);
    }
}

FVulkanTextureView::FVulkanTextureView(FVulkanDevice &InDevice, FRHITextureViewDesc &InDesc, VkImage InImage)
    : Device(InDevice), Desc(InDesc), Image(InImage)
{
    Alloc();
}

FVulkanTextureView::FVulkanTextureView(FVulkanDevice &InDevice, FRHITextureViewDesc &InDesc, VkImage InImage, VkImageView InImageView)
    : Device(InDevice), Desc(InDesc), Image(InImage), View(InImageView)
{
}

FVulkanTextureView::~FVulkanTextureView()
{
    Free();
}

void FVulkanTextureView::Alloc()
{
    VkImageViewCreateInfo CreateInfo = VulkanRHI::GetVkImageViewCreateInfo(Image, Desc.Format, Desc.AspectFlags);

    VkResult Result = VulkanRHI::vkCreateImageView(Device.GetVkDevice(), &CreateInfo, nullptr, &View);
    assert(Result == VK_SUCCESS);
}

void FVulkanTextureView::Free()
{
    VulkanRHI::vkDestroyImageView(Device.GetVkDevice(), View, nullptr);
}