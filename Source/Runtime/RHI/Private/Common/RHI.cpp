#include "Common/RHI.h"

#include "Assertion.h"

RHI* g_rhi = nullptr;
RHIModule* g_rhi_module = nullptr;

void SetRHIModule(RHIInterfaceType InInterfaceType)
{
    if (g_rhi)
    {
        g_rhi->Shutdown();
        delete g_rhi;
        g_rhi = nullptr;
    }

    if (g_rhi_module)
    {
        delete g_rhi_module;
        g_rhi_module = nullptr;
    }

    switch (InInterfaceType)
    {
        case RHIInterfaceType::None: ensure(false && "Missing API backend"); break;
        case RHIInterfaceType::Vulkan: g_rhi_module = create_vulkan_rhi_module(); break;
        case RHIInterfaceType::D3D12: ensure(false && "Missing API backend"); break;
        case RHIInterfaceType::Metal: ensure(false && "Missing API backend"); break;
        case RHIInterfaceType::WebGL: ensure(false && "Missing API backend"); break;
    }

    if (InInterfaceType != RHIInterfaceType::None)
    {
        g_rhi = g_rhi_module->CreateRHI();
    }
}
