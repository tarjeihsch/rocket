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
    ensure(GetTextureDesc().Lifetime != RHIResourceLifetime::Unknown);
}

VulkanTexture::~VulkanTexture()
{
    free();
}

void* VulkanTexture::GetHandle() const
{
    return handle;
}

void VulkanTexture::alloc()
{
    VkImageCreateInfo image_create_info = VulkanRHI::GetVkImageCreateInfo(VkExtent3D(GetTextureDesc().extent[0], GetTextureDesc().extent[1], 1), VulkanRHI::GetVkFormat(GetTextureDesc().Format), VulkanRHI::GetVulkanUsage(GetTextureDesc().Usage));

    if (GetTextureDesc().Lifetime == RHIResourceLifetime::Transient)
    {
        VkResult Result = VulkanRHI::vkCreateImage(get_vulkan_rhi_minimal()->RHIGetVkDevice(), &image_create_info, nullptr, &handle);
        ensure(Result == VK_SUCCESS);

        device.get_transient_resource_allocator()->alloc_image(handle, offset);
    }
    else if (GetTextureDesc().Lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent image
        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult result = vmaCreateImage(device.get_vma_allocator(), &image_create_info, &allocation_create_info, &handle, &allocation_handle, nullptr);
        ensure(result == VK_SUCCESS);
    }

    RHITextureAccessState new_texture_access_state = RHITextureAccessState::Default();
    new_texture_access_state.texture_layout = GetTextureDesc().initial_layout;
    SetTextureAccessState(new_texture_access_state);
}

void VulkanTexture::free()
{
    if (GetTextureDesc().Lifetime == RHIResourceLifetime::Transient)
    {
        // Only destroy handle, memory will be freed externally
        device.get_deferred_deletion_queue()->enqueue_resource(FDeferredDeletionQueue::EType::Image, handle, {});
    }
    else if (GetTextureDesc().Lifetime == RHIResourceLifetime::Persistent)
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

const RHITextureViewDesc& VulkanTextureView::get_desc() const
{
    return desc;
}

void* VulkanTextureView::get_handle() const
{
    return image_view_handle;
}

uint32_t VulkanTextureView::GetBindingIndex() const
{
    return descriptor_handle.Index;
}

RHITexture* VulkanTextureView::get_target_texture() const
{
    return target_texture;
}

void VulkanTextureView::Alloc()
{
    VkImageViewCreateInfo image_view_create_info = VulkanRHI::GetVkImageViewCreateInfo(static_cast<VkImage>(target_texture->GetHandle()), VulkanRHI::GetVkFormat(desc.Format), VulkanRHI::GetVkAspect(desc.aspect));

    VkResult result = VulkanRHI::vkCreateImageView(Device.get_vk_device(), &image_view_create_info, nullptr, &image_view_handle);
    ensure(result == VK_SUCCESS);

    if ((target_texture->GetTextureDesc().Usage & ERHITextureUsage::SRV) == ERHITextureUsage::SRV)
    {
        if (desc.aspect == ERHITextureAspect::Color || desc.aspect == ERHITextureAspect::Depth)
        {
            descriptor_handle = Device.get_descriptor_manager()->add_sampled_image(image_view_handle, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
    }
    else if ((target_texture->GetTextureDesc().Usage & ERHITextureUsage::UAV) == ERHITextureUsage::UAV)
    {
        if (desc.aspect == ERHITextureAspect::Color)
        {
            descriptor_handle = Device.get_descriptor_manager()->add_storage_image(image_view_handle, VK_IMAGE_LAYOUT_GENERAL);
        }
    }
}

void VulkanTextureView::Free()
{
    Device.get_deferred_deletion_queue()->enqueue_resource(FDeferredDeletionQueue::EType::ImageView, image_view_handle, descriptor_handle);
}
