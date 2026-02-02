// Copyright Epic Games, Inc. All Rights Reserved.

#include "VulkanSwapchain.h"

#include "Assertion.h"
#include "Vulkan/VulkanRHIMinimal.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include "VulkanQueue.h"
#include "VulkanSemaphore.h"
#include "VulkanTexture.h"

#include <algorithm>

VulkanSwapchain::VulkanSwapchain(VulkanDevice& InDevice)
	: Device(InDevice),
	NativeHandle(nullptr),
	Extent(0, 0)
{
}

VulkanSwapchain::~VulkanSwapchain()
{
	DestroySwapchain();
}

void VulkanSwapchain::CreateSwapchain(VkSurfaceKHR InSurface, VkSurfaceFormatKHR InSurfaceFormat, VkExtent2D InRequestedExtent)
{
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;
	VulkanRHI::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device.GetVkPhysicalDevice(), InSurface, &SurfaceCapabilities);

	Extent = ClampExtentToSurfaceCapabilities(InRequestedExtent, SurfaceCapabilities);
	const uint32_t ImageCount = DetermineOptimalImageCount(SurfaceCapabilities);

	VkSwapchainCreateInfoKHR CreateInfo = {};
	CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	CreateInfo.surface = InSurface;
	CreateInfo.minImageCount = ImageCount;
	CreateInfo.imageFormat = InSurfaceFormat.format;
	CreateInfo.imageColorSpace = InSurfaceFormat.colorSpace;
	CreateInfo.imageExtent = Extent;
	CreateInfo.imageArrayLayers = 1;
	CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	CreateInfo.preTransform = SurfaceCapabilities.currentTransform;
	CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	CreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	CreateInfo.clipped = VK_TRUE;
	CreateInfo.oldSwapchain = VK_NULL_HANDLE;

	ConfigureQueueFamilySharing(CreateInfo);

	VkResult Result = VulkanRHI::vkCreateSwapchainKHR(Device.GetVkDevice(), &CreateInfo, nullptr, &NativeHandle);
	ensure(Result == VK_SUCCESS, "Failed to create swapchain.");

	// Retrieve swapchain images
	uint32_t ActualImageCount = 0;
	VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), NativeHandle, &ActualImageCount, nullptr);

	std::vector<VkImage> SwapchainImages(ActualImageCount);
	VulkanRHI::vkGetSwapchainImagesKHR(Device.GetVkDevice(), NativeHandle, &ActualImageCount, SwapchainImages.data());

	RK_LOG_INFO("Swapchain created: {}x{}, {} images", Extent.width, Extent.height, ActualImageCount);

	CreateBackBufferTextures(SwapchainImages);
	CreateSynchronizationPrimitives(ActualImageCount);
}

void VulkanSwapchain::DestroySwapchain()
{
	if (NativeHandle == VK_NULL_HANDLE)
	{
		return;
	}

	Device.WaitUntilIdle();

	BackBuffers.clear();
	AcquireSyncStates.clear();
	ImageSyncStates.clear();

	VulkanRHI::vkDestroySwapchainKHR(Device.GetVkDevice(), NativeHandle, nullptr);
	NativeHandle = VK_NULL_HANDLE;

	ResetSyncState();
}

bool VulkanSwapchain::AcquireNextImage(VulkanQueue& Queue)
{
	CurrentAcquireIndex = TotalAcquireCount % AcquireSyncStates.size();
	AcquireSyncState& AcquireState = AcquireSyncStates[CurrentAcquireIndex];

	if (AcquireState.LastUsedTimelineValue > 0)
	{
		Queue.WaitForTimelineValue(AcquireState.LastUsedTimelineValue);
	}

	uint32_t ImageIndex = 0;
	VkResult Result = VulkanRHI::vkAcquireNextImageKHR(Device.GetVkDevice(), NativeHandle, UINT64_MAX, AcquireState.Semaphore->NativeHandle, VK_NULL_HANDLE, &ImageIndex);

	if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR)
	{
		return false;
	}

	ensure(Result == VK_SUCCESS, "Failed to acquire swapchain image.");

	CurrentImageIndex = ImageIndex;
	TotalAcquireCount++;

	ImageSyncState& ImageState = ImageSyncStates[CurrentImageIndex];
	if (ImageState.TimelineValue > 0)
	{
		Queue.WaitForTimelineValue(ImageState.TimelineValue);
		ImageState.TimelineValue = 0;
	}

	return true;
}

void VulkanSwapchain::Present(VulkanQueue& Queue)
{
	ImageSyncState& ImageState = ImageSyncStates[CurrentImageIndex];

	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.waitSemaphoreCount = 1;
	PresentInfo.pWaitSemaphores = &ImageState.Semaphore->NativeHandle;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &NativeHandle;
	PresentInfo.pImageIndices = &CurrentImageIndex;

	VkResult Result = VulkanRHI::vkQueuePresentKHR(Device.GetPresentQueue()->GetHandle(), &PresentInfo);

	// VK_SUBOPTIMAL_KHR is not a fatal error - we can continue presenting.
	// The next BeginFrame will detect out-of-date and trigger recreation.
	if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR && Result != VK_ERROR_OUT_OF_DATE_KHR)
	{
		ensure(false, "Failed to present swapchain image.");
	}

	// Record synchronization state for this frame
	const uint64_t SubmittedTimelineValue = Queue.GetLastSubmittedTimelineValue();

	ImageState.TimelineValue = SubmittedTimelineValue;

	AcquireSyncStates[CurrentAcquireIndex].LastUsedTimelineValue = SubmittedTimelineValue;

	TotalPresentCount++;
}

////////////////////////////////////////////////////////////////////////////////
// Back Buffer Access
////////////////////////////////////////////////////////////////////////////////

RHITexture* VulkanSwapchain::GetBackBufferTexture() const
{
	ensure(!BackBuffers.empty(), "Swapchain has no back buffers.");
	return BackBuffers[CurrentImageIndex].get();
}

////////////////////////////////////////////////////////////////////////////////
// Internal Helpers
////////////////////////////////////////////////////////////////////////////////

VkExtent2D VulkanSwapchain::ClampExtentToSurfaceCapabilities(
	VkExtent2D InExtent,
	const VkSurfaceCapabilitiesKHR& Capabilities) const
{
	// Special value indicating the surface size is determined by the swapchain extent
	if (Capabilities.currentExtent.width != UINT32_MAX)
	{
		return Capabilities.currentExtent;
	}

	VkExtent2D ClampedExtent;
	ClampedExtent.width = std::clamp(
		InExtent.width,
		Capabilities.minImageExtent.width,
		Capabilities.maxImageExtent.width
	);
	ClampedExtent.height = std::clamp(
		InExtent.height,
		Capabilities.minImageExtent.height,
		Capabilities.maxImageExtent.height
	);

	return ClampedExtent;
}

uint32_t VulkanSwapchain::DetermineOptimalImageCount(const VkSurfaceCapabilitiesKHR& Capabilities) const
{
	// Request one more than minimum to allow triple buffering when possible
	uint32_t DesiredCount = Capabilities.minImageCount + 1;

	// Clamp to maximum if one is specified (0 means unlimited)
	if (Capabilities.maxImageCount > 0)
	{
		DesiredCount = min(DesiredCount, Capabilities.maxImageCount);
	}

	return DesiredCount;
}

VkSharingMode VulkanSwapchain::ConfigureQueueFamilySharing(VkSwapchainCreateInfoKHR& OutCreateInfo) const
{
	const uint32_t GraphicsFamily = GetVulkanRHIMinimal()->RHIGetGraphicsQueueFamilyIndex();
	const uint32_t PresentFamily = GetVulkanRHIMinimal()->RHIGetPresentQueueFamilyIndex();

	if (GraphicsFamily != PresentFamily)
	{
		// Different queue families require concurrent sharing mode.
		// This has slightly lower performance but avoids explicit ownership transfers.
		static std::vector<uint32_t> QueueFamilyIndices;
		QueueFamilyIndices = { GraphicsFamily, PresentFamily };

		OutCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		OutCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>(QueueFamilyIndices.size());
		OutCreateInfo.pQueueFamilyIndices = QueueFamilyIndices.data();

		return VK_SHARING_MODE_CONCURRENT;
	}

	OutCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	OutCreateInfo.queueFamilyIndexCount = 0;
	OutCreateInfo.pQueueFamilyIndices = nullptr;

	return VK_SHARING_MODE_EXCLUSIVE;
}

void VulkanSwapchain::CreateBackBufferTextures(const std::vector<VkImage>& Images)
{
	BackBuffers.clear();
	BackBuffers.reserve(Images.size());

	const RHITextureDesc BackBufferDesc = RHITextureDesc::Create2D(
		Extent.width,
		Extent.height,
		RHIResourceLifetime::Unmanaged,
		RHITextureFormat::B8G8R8A8_SRGB,
		RHITextureUsage::Color | RHITextureUsage::CopyDst | RHITextureUsage::Present,
		RHITextureLayout::None,
		0, 0, 1
	);

	for (VkImage Image : Images)
	{
		BackBuffers.push_back(std::make_unique<VulkanTexture>(Device, BackBufferDesc, Image));
	}
}

void VulkanSwapchain::CreateSynchronizationPrimitives(uint32_t InImageCount)
{
	AcquireSyncStates.clear();
	AcquireSyncStates.resize(InImageCount);

	for (AcquireSyncState& State : AcquireSyncStates)
	{
		State.Semaphore = std::make_shared<VulkanSemaphore>(Device);
		State.LastUsedTimelineValue = 0;
	}

	ImageSyncStates.clear();
	ImageSyncStates.resize(InImageCount);

	for (ImageSyncState& State : ImageSyncStates)
	{
		State.Semaphore = std::make_shared<VulkanSemaphore>(Device);
		State.TimelineValue = 0;
	}
}

void VulkanSwapchain::ResetSyncState()
{
	CurrentAcquireIndex = 0;
	CurrentImageIndex = 0;
	TotalAcquireCount = 0;
	TotalPresentCount = 0;
}