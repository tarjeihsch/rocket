#pragma once

#include "../../../Runtime/IMGUIModule/Public/Dockspace.h"

class RMenuBar : public RDockspace
{
public:
    RMenuBar();

    virtual void UpdateDockspacePanel() override;
};