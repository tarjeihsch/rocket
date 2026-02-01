#include "VulkanTexture.h"

#include "Assertion.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanTextureView.h"

VulkanTexture::VulkanTexture(VulkanDevice &in_device, const RHITextureDesc &in_desc)
    : RHITexture(in_desc), device(in_device), AllocationHandle(nullptr)
{
    alloc();
}

VulkanTexture::VulkanTexture(VulkanDevice &in_device, const RHITextureDesc &in_desc, VkImage in_image_handle)
    : RHITexture(in_desc), device(in_device), NativeHandle(in_image_handle), AllocationHandle(nullptr)
{
    ensure(GetTextureDesc().lifetime != RHIResourceLifetime::Unknown);
}

VulkanTexture::~VulkanTexture()
{
    free();
}

void* VulkanTexture::GetNativeTexture() const
{
    return NativeHandle;
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
        VkResult Result = VulkanRHI::vkCreateImage(GetVulkanRHIMinimal()->RHIGetVkDevice(), &image_create_info, nullptr, &NativeHandle);
        ensure(Result == VK_SUCCESS);

        device.GetTransientMemoryAllocator()->AllocImage(NativeHandle, offset);
    }
    else if (GetTextureDesc().lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, allocate a persistent image
        VmaAllocationCreateInfo allocation_create_info = {};
        allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        VkResult result = vmaCreateImage(device.GetPersistentMemoryAllocator(), &image_create_info, &allocation_create_info, &NativeHandle, &AllocationHandle, nullptr);
        ensure(result == VK_SUCCESS);
    }

#if RK_DEBUG
    VkDebugUtilsObjectNameInfoEXT DebugUtilInfo = {};
    DebugUtilInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    DebugUtilInfo.objectType = VK_OBJECT_TYPE_IMAGE;
    DebugUtilInfo.objectHandle = (uint64_t)NativeHandle;
    DebugUtilInfo.pObjectName = GetTextureDesc().DebugName;
    VulkanRHI::vkSetDebugUtilsObjectNameEXT(device.GetVkDevice(), &DebugUtilInfo );
#endif
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
        device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::Image, NativeHandle, {});
    }
    else if (GetTextureDesc().lifetime == RHIResourceLifetime::Persistent)
    {
        // We own the allocation, free memory
        device.GetDeferredDeletionQueue()->enqueue_resource_with_allocation(FDeferredDeletionQueue::EType::Image, NativeHandle, {}, AllocationHandle);
    }
}