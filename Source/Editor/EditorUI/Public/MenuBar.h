#pragma once

#include "Dockspace.h"

class RMenuBar : public RDockspace
{
public:
    RMenuBar();

    virtual void UpdateDockspacePanel() override;
};