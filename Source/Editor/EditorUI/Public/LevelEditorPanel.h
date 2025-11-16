#pragma once

#include "Panel.h"

class RLevelEditorPanel : public RPanel
{
public:
    explicit RLevelEditorPanel();

protected:
    virtual void UpdatePanelImpl() override;
};