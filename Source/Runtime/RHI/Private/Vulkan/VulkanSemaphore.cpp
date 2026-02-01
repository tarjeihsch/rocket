#include "VulkanSemaphore.h"

#include "VulkanDevice.h"
#include "VulkanLoader.h"

VulkanSemaphore::VulkanSemaphore(VulkanDevice& InDevice)
	: Device(InDevice)
{
	VkSemaphoreCreateInfo SemaphoreInfo = {};
	SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	SemaphoreInfo.flags = 0;
	VulkanRHI::vkCreateSemaphore(Device.GetVkDevice(), &SemaphoreInfo, nullptr, &NativeHandle);
}

VulkanSemaphore::~VulkanSemaphore()
{
	VulkanRHI::vkDestroySemaphore(Device.GetVkDevice(), NativeHandle, nullptr);
}
