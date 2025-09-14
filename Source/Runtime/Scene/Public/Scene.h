#pragma once 

#include "Camera.h"
#include "Delegate.h"
#include "Registry.h"

class CScene final
{
public:
    void Init();
    void Cleanup();

    void SetActiveCamera(ICamera* InCamera)
    {
        ActiveCamera = InCamera;
    }

    [[nodiscard]] ICamera* GetActiveCamera() const
    {
	return ActiveCamera;
    }

    [[nodiscard]] PRegistry* GetRegistry() const
    {
	return Registry;
    }

private:
    ICamera* ActiveCamera;

    PRegistry* Registry;
};