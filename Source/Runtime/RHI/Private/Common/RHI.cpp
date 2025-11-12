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
        case ERHIInterfaceType::None: ensure(false && "Missing API backend"); break;
        case ERHIInterfaceType::Vulkan: GRHIModule = CreateVulkanRHIModule(); break;
        case ERHIInterfaceType::D3D12: ensure(false && "Missing API backend"); break;
        case ERHIInterfaceType::Metal: ensure(false && "Missing API backend"); break;
        case ERHIInterfaceType::WebGL: ensure(false && "Missing API backend"); break;
    }

    if (InInterfaceType != ERHIInterfaceType::None)
    {
        GRHI = GRHIModule->CreateRHI();
    }
}
