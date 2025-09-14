#include "Common/RHI.h"

#include "Assert.h"

IRHI* G_RHI = nullptr;
IRHIModule* GRHIModule = nullptr;

void SetRHIModule(ERHIInterfaceType InInterfaceType)
{
    if (G_RHI)
    {
        G_RHI->Shutdown();
        delete G_RHI;
        G_RHI = nullptr;
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
        case ERHIInterfaceType::None: RK_ASSERT("Missing API backend"); break;
    }

    G_RHI = GRHIModule->CreateRHI();
}