#pragma once

#include "Widget.h"

class RDockspace : public RWidget
{
public:
    RDockspace();
    explicit RDockspace(const char* Name);

    virtual void PreUpdate() override;
    virtual void UpdateImpl() override;
    virtual void UpdateDockspacePanel() {}
};