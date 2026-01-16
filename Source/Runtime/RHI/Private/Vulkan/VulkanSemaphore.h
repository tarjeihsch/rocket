#pragma once

#include "Common/RHISemaphore.h"

#include "vulkan/vulkan_core.h"

struct VulkanSemaphore final : RHISemaphore
{
    explicit VulkanSemaphore(VkSemaphore InSemaphore)
        : Handle(InSemaphore)
    {
    }

    void* GetNativeHandle() override { return Handle; };

    VkSemaphore Handle = VK_NULL_HANDLE;
};
