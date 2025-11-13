#pragma once

#include "Panel.h"

class RSceneHierarchyPanel : public IPanel
{
public:
    virtual const char* GetName() const override;
	virtual void Draw(float DeltaTime) override;
};