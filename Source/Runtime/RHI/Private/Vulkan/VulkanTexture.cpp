#include "VulkanTexture.h"

#include "Assertion.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "Vulkan/VulkanRHIMinimal.h"

VulkanTexture::VulkanTexture(VulkanDevice &in_device, RHITextureDesc &in_desc)
    : RHITexture(in_desc, RHITextureAccessState::Default()), device(in_device), allocation_handle(nullptr)
{
    alloc();
}

VulkanTexture::VulkanTexture(VulkanDevice &in_device, RHITextureDesc &in_desc, VkImage in_image_handle)
    : RHITexture(in_desc, RHITextureAccessState::Default()), device(in_device), handle(in_image_handle), allocation_handle(nullptr)
{
    ensure(GetTextureDesc().lifetime != RHIResourceLifetime::Unknown);
}

VulkanTexture::~VulkanTexture()
{
    free();
}

void* VulkanTexture::GetNativeTexture() const
{
    return handle;
}

RHITextureView* VulkanTexture::GetDefaultView()
{
    if (default_view)
    {
        return default_view;
    }

    RHITextureAspect aspect;
	switch (GetTextureDesc().format)
	{
	    case RHITextureFormat::D32_SFLOAT:
	    case RHITextureFormat::D32_SFLOAT_S8_UINT:
	    {
	    	aspect = RHITextureAspect::DepthStencil;
	    	break;
	    }
	    default: aspect = RHITextureAspect::Color;
	}

	RHITextureViewDesc Desc = {};
	Desc.Format = GetTextureDesc().format;
	Desc.aspect = aspect;
	Desc.BaseMipLevel = 0;
	Desc.NumMips = GetTextureDesc().NumMips;
	Desc.BaseArrayLayer = 0;
	Desc.NumArrayLayers = GetTextureDesc().LayerCount;

    default_view = new VulkanTextureView(device, Desc, this);
    return default_view;
}

void VulkanTexture::alloc()
{
    VkImageCreateInfo image_create_info = VulkanRHI::GetVkImageCreateInfo(VkExtent3D(GetTextureDesc().extent[0], GetTextureDesc().extent[1], 1), VulkanRHI::GetVkFormat(GetTextureDesc().format), VulkanRHI::GetVulkanUsage(GetTextureDesc().usage));

    if (GetTextureDesc().lifetime == RHIResourceLifetime::Transient)
    {
        VkResult Result = VulkanRHI::vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &image_create_info, nullptr, &handle);
        ensure(Result == VK_SUCCESS);

        device.GetTransientMemoryAllocator()->AllocImage(handle, offset);
    }
    else if (GetTextureDesc().lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent image
        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult result = vmaCreateImage(device.get_vma_allocator(), &image_create_info, &allocation_create_info, &handle, &allocation_handle, nullptr);
        ensure(result == VK_SUCCESS);
    }
}

void VulkanTexture::free()
{
	if (default_view)
	{
		delete default_view;
	}

    if (GetTextureDesc().lifetime == RHIResourceLifetime::Transient)
    {
        // Only destroy handle, memory will be freed externally
        device.get_deferred_deletion_queue()->enqueue_resource(FDeferredDeletionQueue::EType::Image, handle, {});
    }
    else if (GetTextureDesc().lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, free memory
        device.get_deferred_deletion_queue()->enqueue_resource_with_allocation(FDeferredDeletionQueue::EType::Image, handle, {}, allocation_handle);
    }
}

VulkanTextureView::VulkanTextureView(VulkanDevice &InDevice, RHITextureViewDesc &InDesc, RHITexture* InTexture)
    : Device(InDevice), target_texture(InTexture), desc(InDesc)
{
    Alloc();
}

VulkanTextureView::VulkanTextureView(VulkanDevice &InDevice, RHITextureViewDesc &InDesc, RHITexture* InTexture, VkImageView InImageView)
    : Device(InDevice), image_view_handle(InImageView), target_texture(InTexture), desc(InDesc)
{
}

VulkanTextureView::~VulkanTextureView()
{
    Free();
}

const RHITextureViewDesc& VulkanTextureView::GetDesc() const
{
    return desc;
}

void* VulkanTextureView::GetNativeHandle() const
{
    return image_view_handle;
}

uint32_t VulkanTextureView::GetBindingIndex() const
{
    return bindlessHandle.Index;
}

RHITexture* VulkanTextureView::GetRHITexture() const
{
    return target_texture;
}

void VulkanTextureView::Alloc()
{
    VkImageViewCreateInfo image_view_create_info = VulkanRHI::GetVkImageViewCreateInfo(static_cast<VkImage>(target_texture->GetNativeTexture()), VulkanRHI::GetVkFormat(desc.Format), VulkanRHI::GetVkAspect(desc.aspect));

    VkResult result = VulkanRHI::vkCreateImageView(Device.GetVkDevice(), &image_view_create_info, nullptr, &image_view_handle);
    ensure(result == VK_SUCCESS);

    if ((target_texture->GetTextureDesc().usage & RHITextureUsage::SRV) == RHITextureUsage::SRV)
    {
        if (desc.aspect == RHITextureAspect::Color || desc.aspect == RHITextureAspect::Depth)
        {
            bindlessHandle = Device.GetBindlessDescriptorManager()->GetHeap().RegisterTexture(image_view_handle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
    else if ((target_texture->GetTextureDesc().usage & RHITextureUsage::UAV) == RHITextureUsage::UAV)
    {
        if (desc.aspect == RHITextureAspect::Color)
        {
            bindlessHandle = Device.GetBindlessDescriptorManager()->GetHeap().RegisterStorageImage(image_view_handle, VK_IMAGE_LAYOUT_GENERAL);
        }
    }
}

void VulkanTextureView::Free()
{
    Device.get_deferred_deletion_queue()->enqueue_resource(FDeferredDeletionQueue::EType::ImageView, image_view_handle, bindlessHandle);
}
