#include "VulkanTexture.h"

#include "Assert.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanResourceManager.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDescriptorManager.h"

#include <cassert>

#include "VulkanDescriptorManager.h"

FVulkanTexture::FVulkanTexture(FVulkanDevice& InDevice)
    : Device(InDevice)
{
}

FVulkanTexture::FVulkanTexture(FVulkanDevice &InDevice, FRHITextureDesc &InDesc)
    : Device(InDevice), Desc(InDesc), Allocation(nullptr)
{
    Alloc();
}

FVulkanTexture::FVulkanTexture(FVulkanDevice &InDevice, FRHITextureDesc &InDesc, VkImage InImage)
    : Device(InDevice), Desc(InDesc), Image(InImage), Allocation(nullptr)
{
    assert(Desc.Lifetime != ERHIResourceLifetime::Unknown);
}

FVulkanTexture::~FVulkanTexture()
{
    Free();
}

const FRHITextureDesc& FVulkanTexture::RHIGetTextureDesc() const
{
    return Desc;
}

void* FVulkanTexture::GetHandle() const
{
    return Image;
}

void FVulkanTexture::Alloc()
{
    VkImageCreateInfo CreateInfo = VulkanRHI::GetVkImageCreateInfo(VkExtent3D(Desc.Tex2D.Extent[0], Desc.Tex2D.Extent[1], 1), VulkanRHI::GetVkFormat(Desc.Format), VulkanRHI::GetVulkanUsage(Desc.Usage));

    if (Desc.Lifetime == ERHIResourceLifetime::Transient)
    {
        VkResult Result = VulkanRHI::vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CreateInfo, nullptr, &Image);
        RK_ASSERT(Result == VK_SUCCESS);

        Device.GetTransientResourceAllocator()->AllocImage(Image, Offset);
    }
    else if (Desc.Lifetime == ERHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent image
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult Result = vmaCreateImage(Device.GetVmaAllocator(), &CreateInfo, &AllocationCreateInfo, &Image, &Allocation, nullptr);
        RK_ASSERT(Result == VK_SUCCESS);
    }

    CurrentLayout = Desc.InitialLayout;
}

void FVulkanTexture::Free()
{
    if (Desc.Lifetime == ERHIResourceLifetime::Transient)
    {
        // Only destroy handle, memory will be freed externally
        Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::Image, Image, {});
    }
    else if (Desc.Lifetime == ERHIResourceLifetime::Persistent)
    {
        // We own the allocation, free memory
        Device.GetDeferredDeletionQueue()->EnqueueResourceWithAllocation(FDeferredDeletionQueue::EType::Image, Image, {}, Allocation);
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

const FRHITextureViewDesc& FVulkanTextureView::RHIGetTextureViewDesc() const
{
    return Desc;
}

void* FVulkanTextureView::GetHandle() const
{
    return View;
}

void FVulkanTextureView::Alloc()
{
    VkImageViewCreateInfo CreateInfo = VulkanRHI::GetVkImageViewCreateInfo(Image, VulkanRHI::GetVkFormat(Desc.Format), VulkanRHI::GetVkAspect(Desc.Aspect));

    VkResult Result = VulkanRHI::vkCreateImageView(Device.GetVkDevice(), &CreateInfo, nullptr, &View);
    assert(Result == VK_SUCCESS);
}

void FVulkanTextureView::Free()
{
    //VulkanRHI::vkDestroyImageView(Device.GetVkDevice(), View, nullptr);
    Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::ImageView, View, m_DescriptorHandle);
}