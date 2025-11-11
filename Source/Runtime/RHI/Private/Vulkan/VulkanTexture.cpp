#include "VulkanTexture.h"

#include "Assertion.h"
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
    VkImageCreateInfo CreateInfo = VulkanRHI::GetVkImageCreateInfo(VkExtent3D(Desc.Extent[0], Desc.Extent[1], 1), VulkanRHI::GetVkFormat(Desc.Format), VulkanRHI::GetVulkanUsage(Desc.Usage));

    if (Desc.Lifetime == ERHIResourceLifetime::Transient)
    {
        VkResult Result = VulkanRHI::vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &CreateInfo, nullptr, &Image);
        ensure(Result == VK_SUCCESS);

        Device.GetTransientResourceAllocator()->AllocImage(Image, Offset);
    }
    else if (Desc.Lifetime == ERHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent image
        VmaAllocationCreateInfo AllocationCreateInfo = {};
        AllocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult Result = vmaCreateImage(Device.GetVmaAllocator(), &CreateInfo, &AllocationCreateInfo, &Image, &Allocation, nullptr);
        ensure(Result == VK_SUCCESS);
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

FVulkanTextureView::FVulkanTextureView(FVulkanDevice &InDevice, FRHITextureViewDesc &InDesc, FRHITexture* InTexture)
    : Device(InDevice), Desc(InDesc), Texture(InTexture)
{
    Alloc();
}

FVulkanTextureView::FVulkanTextureView(FVulkanDevice &InDevice, FRHITextureViewDesc &InDesc, FRHITexture* InTexture, VkImageView InImageView)
    : Device(InDevice), Desc(InDesc), Texture(InTexture), ImageView(InImageView)
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
    return ImageView;
}
uint32_t FVulkanTextureView::GetBindingIndex() const
{
    return DescriptorHandle.Index;
}

void FVulkanTextureView::Alloc()
{
    VkImageViewCreateInfo CreateInfo = VulkanRHI::GetVkImageViewCreateInfo(static_cast<VkImage>(Texture->GetHandle()), VulkanRHI::GetVkFormat(Desc.Format), VulkanRHI::GetVkAspect(Desc.Aspect));

    VkResult Result = VulkanRHI::vkCreateImageView(Device.GetVkDevice(), &CreateInfo, nullptr, &ImageView);
    assert(Result == VK_SUCCESS);

    if ((Texture->RHIGetTextureDesc().Usage & ERHITextureUsage::SRV) == ERHITextureUsage::SRV)
    {
        if (Desc.Aspect == ERHITextureAspect::Color || Desc.Aspect == ERHITextureAspect::Depth)
        {
            DescriptorHandle = Device.GetDescriptorManager()->AddSampledImage(
                ImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
    else if ((Texture->RHIGetTextureDesc().Usage & ERHITextureUsage::UAV) == ERHITextureUsage::UAV)
    {
        if (Desc.Aspect == ERHITextureAspect::Color)
        {
            DescriptorHandle = Device.GetDescriptorManager()->AddStorageImage(
                ImageView, VK_IMAGE_LAYOUT_GENERAL);
        }
    }
}

void FVulkanTextureView::Free()
{
    Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::ImageView, ImageView, DescriptorHandle);
}
