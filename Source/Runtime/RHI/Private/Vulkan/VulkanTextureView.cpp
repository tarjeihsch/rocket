#include "VulkanTextureView.h"

#include "Assertion.h"
#include "VulkanTexture.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"

VulkanTextureView::VulkanTextureView(VulkanDevice& InDevice, const RHITextureViewDesc& InDesc, RHITexture* InTexture)
	: Device(InDevice), target_texture(InTexture), desc(InDesc)
{
	Alloc();
}

VulkanTextureView::VulkanTextureView(VulkanDevice& InDevice, const RHITextureViewDesc& InDesc, RHITexture* InTexture, VkImageView InImageView)
	: Device(InDevice), NativeHandle(InImageView), target_texture(InTexture), desc(InDesc)
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
	return NativeHandle;
}

RHIDescriptorHandle VulkanTextureView::GetDescriptorHandle() const
{
	return DescriptorHandle;
}

RHITexture* VulkanTextureView::GetRHITexture() const
{
	return target_texture;
}

void VulkanTextureView::Alloc()
{
	VkImageViewCreateInfo image_view_create_info = VulkanRHI::GetVkImageViewCreateInfo(static_cast<VkImage>(target_texture->GetNativeTexture()), VulkanRHI::GetVkFormat(desc.Format), VulkanRHI::GetVkAspect(desc.aspect));

	VkResult result = VulkanRHI::vkCreateImageView(Device.GetVkDevice(), &image_view_create_info, nullptr, &NativeHandle);
	ensure(result == VK_SUCCESS);

	if ((target_texture->GetTextureDesc().usage & RHITextureUsage::SRV) == RHITextureUsage::SRV)
	{
		if (desc.aspect == RHITextureAspect::Color || desc.aspect == RHITextureAspect::Depth)
		{
			DescriptorHandle = Device.GetBindlessDescriptorManager()->GetHeap().RegisterTexture(NativeHandle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
	}
	else if ((target_texture->GetTextureDesc().usage & RHITextureUsage::UAV) == RHITextureUsage::UAV)
	{
		if (desc.aspect == RHITextureAspect::Color)
		{
			DescriptorHandle = Device.GetBindlessDescriptorManager()->GetHeap().RegisterStorageImage(NativeHandle, VK_IMAGE_LAYOUT_GENERAL);
		}
	}
}

void VulkanTextureView::Free()
{
	Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::ImageView, NativeHandle, GetDescriptorHandle());
}