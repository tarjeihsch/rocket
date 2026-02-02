#pragma once

#include "Common/RHISemaphore.h"

#include "vulkan/vulkan_core.h"

class VulkanDevice;

class VulkanSemaphore final : public RHISemaphore
{
    VulkanDevice& Device;

public:
    VulkanSemaphore(VulkanDevice& InDevice);
    ~VulkanSemaphore();

    void* GetNativeHandle() override 
    { 
        return NativeHandle; 
    };

    VkSemaphore NativeHandle = VK_NULL_HANDLE;
};
