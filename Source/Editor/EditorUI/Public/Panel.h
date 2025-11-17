#pragma once

#include "Widget.h"

class RPanel : public RWidget
{
public:
    explicit RPanel();

protected:
    virtual void UpdateImpl() override;
};