#include "Common/RHI.h"

RHI::IRHI *CreateVulkanRHI();

RHI::IRHI* RHI::g_rhi = nullptr;

void RHI::Initialize(GPUDriver driver)
{
    if (g_rhi)
    {
        delete g_rhi;
    }

    switch (driver)
    {
        case RHI::GPUDriver::Vulkan: g_rhi = ::CreateVulkanRHI();
    }
}

void RHI::Shutdown()
{
    delete g_rhi;
}