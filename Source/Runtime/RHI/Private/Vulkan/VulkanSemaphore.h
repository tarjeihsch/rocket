#pragma once

#include "Common/RHISemaphore.h"

#include "vulkan/vulkan_core.h"

struct FVulkanSemaphore final : RHISemaphore
{
    explicit FVulkanSemaphore(VkSemaphore InSemaphore)
        : Handle(InSemaphore)
    {
    }

    void* GetHandle() override { return Handle; };

    VkSemaphore Handle = VK_NULL_HANDLE;
};
