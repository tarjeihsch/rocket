#pragma once

#include "Common/RHISemaphore.h"

#include "vulkan/vulkan_core.h"

struct FVulkanSemaphore final : FRHISemaphore
{
    explicit FVulkanSemaphore(VkSemaphore In)
        : Handle(In)
    {
    }

    VkSemaphore Handle = VK_NULL_HANDLE;
};
