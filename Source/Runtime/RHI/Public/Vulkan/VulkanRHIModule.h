#pragma once

#include "Common/RHI.h"

class FVulkanRHIModule : public RHIModule
{
public:
    virtual RHI* CreateRHI() override;
};