#pragma once

#include "Common/RHI.h"

class FVulkanRHIModule : public IRHIModule
{
public:
    virtual IRHI* CreateRHI() override;
};