#include "Vulkan/VulkanRHIModule.h"

#include "VulkanRHI.h"

IRHI* FVulkanRHIModule::CreateRHI()
{
    return new CVulkanRHI();
}

IRHIModule* CreateVulkanRHIModule()
{
    return new FVulkanRHIModule();
}
