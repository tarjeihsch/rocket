#include "Vulkan/VulkanRHIModule.h"

#include "VulkanRHI.h"

RHI* FVulkanRHIModule::CreateRHI()
{
    return new CVulkanRHI();
}

RHIModule* create_vulkan_rhi_module()
{
    return new FVulkanRHIModule();
}
