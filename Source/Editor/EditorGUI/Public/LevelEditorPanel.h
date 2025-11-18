#pragma once

#include "../../../Runtime/IMGUIModule/Public/Widget.h"

class RLevelEditorPanel : public RWidget
{
public:
    explicit RLevelEditorPanel();

protected:
    virtual void UpdateImpl() override;
};