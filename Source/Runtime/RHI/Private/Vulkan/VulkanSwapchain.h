#pragma once

#include "Common/RHISwapchain.h"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

class VulkanDevice;
class VulkanQueue;
struct VulkanSemaphore;
struct VulkanTexture;

class VulkanSwapchain : public RHISwapchain
{
	friend class VulkanSurface;

	VulkanDevice& Device;

public:
	explicit VulkanSwapchain(VulkanDevice& InDevice);
	virtual ~VulkanSwapchain() override;

	VkSwapchainKHR GetHandle() const { return NativeHandle; }
	VkExtent2D GetExtent() const { return Extent; }
	uint32_t GetImageCount() const { return static_cast<uint32_t>(BackBuffers.size()); }
	uint32_t GetCurrentImageIndex() const { return CurrentImageIndex; }
	RHITexture* GetBackBufferTexture() const;
	
	inline std::shared_ptr<VulkanSemaphore> GetImageAvailableSemaphore() const;
	inline std::shared_ptr<VulkanSemaphore> GetRenderFinishedSemaphore() const;

private:
	void CreateSwapchain(VkSurfaceKHR InSurface, VkSurfaceFormatKHR InSurfaceFormat, VkExtent2D InRequestedExtent);
	void DestroySwapchain();

	bool AcquireNextImage(VulkanQueue& Queue);
	void Present(VulkanQueue& Queue);

private:
	struct AcquireSyncState
	{
		std::shared_ptr<VulkanSemaphore> Semaphore;
		uint64_t LastUsedTimelineValue = 0;
	};

	struct ImageSyncState
	{
		std::shared_ptr<VulkanSemaphore> Semaphore;
		uint64_t TimelineValue = 0;
	};

	VkExtent2D ClampExtentToSurfaceCapabilities(VkExtent2D InExtent, const VkSurfaceCapabilitiesKHR& Capabilities) const;
	uint32_t DetermineOptimalImageCount(const VkSurfaceCapabilitiesKHR& Capabilities) const;
	VkSharingMode ConfigureQueueFamilySharing(VkSwapchainCreateInfoKHR& OutCreateInfo) const;

	void CreateBackBufferTextures(const std::vector<VkImage>& Images);
	void CreateSynchronizationPrimitives(uint32_t InImageCount);
	void ResetSyncState();

private:	
	VkSwapchainKHR NativeHandle;
	VkExtent2D Extent;
	
	std::vector<std::unique_ptr<VulkanTexture>> BackBuffers;
	std::vector<AcquireSyncState> AcquireSyncStates;
	std::vector<ImageSyncState> ImageSyncStates;

	uint32_t CurrentAcquireIndex = 0;
	uint32_t CurrentImageIndex = 0;
	
	uint64_t TotalAcquireCount = 0;
	uint64_t TotalPresentCount = 0;
};

inline std::shared_ptr<VulkanSemaphore> VulkanSwapchain::GetImageAvailableSemaphore() const
{
	return AcquireSyncStates[CurrentAcquireIndex].Semaphore;
}

inline std::shared_ptr<VulkanSemaphore> VulkanSwapchain::GetRenderFinishedSemaphore() const
{
	return ImageSyncStates[CurrentImageIndex].Semaphore;
}