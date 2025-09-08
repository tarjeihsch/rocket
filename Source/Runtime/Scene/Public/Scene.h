#pragma once 

#include "Camera.h"
#include "Component.h"
#include "Registry.h"

class PScene
{
public:
    void Init()
    {
	    Registry = new PRegistry();
    }

    void Cleanup()
    {
	    delete Registry;
    }

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

    void OnImGuiRender();

private:
    ICamera* ActiveCamera;

    PRegistry* Registry;
};