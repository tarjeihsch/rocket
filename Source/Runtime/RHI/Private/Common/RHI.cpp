#include "Common/RHI.h"

#include "Assertion.h"

IRHI* GRHI = nullptr;
IRHIModule* GRHIModule = nullptr;

void SetRHIModule(ERHIInterfaceType InInterfaceType)
{
    if (GRHI)
    {
        GRHI->Shutdown();
        delete GRHI;
        GRHI = nullptr;
    }

    if (GRHIModule)
    {
        delete GRHIModule;
        GRHIModule = nullptr;
    }

    switch (InInterfaceType)
    {
        case ERHIInterfaceType::Vulkan: GRHIModule = CreateVulkanRHIModule(); break;
        case ERHIInterfaceType::D3D12: RK_ASSERT("Missing API backend"); break;
        case ERHIInterfaceType::Metal: RK_ASSERT("Missing API backend"); break;
        case ERHIInterfaceType::WebGL: RK_ASSERT("Missing API backend"); break;
    }

    if (InInterfaceType != ERHIInterfaceType::None)
    {
        GRHI = GRHIModule->CreateRHI();
    }
}
