#pragma once

#include "Widget.h"

class RActor;

class RSceneHierarchyPanel : public RWidget
{
public:
    explicit RSceneHierarchyPanel();

protected:
    virtual void UpdateImpl() override;

private:
    RActor* SelectedActor;
};