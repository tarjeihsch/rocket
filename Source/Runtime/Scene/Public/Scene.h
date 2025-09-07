#pragma once 

#include "Camera.h"
#include "Registry.h"

class PScene
{
public:
    void Init()
    {
	    Camera = new CPerspectiveCamera();
	    Registry = new PRegistry();
    }

    void Cleanup()
    {
	    delete Camera;
	    delete Registry;
    }

    [[nodiscard]] CPerspectiveCamera* GetCamera() const
    {
	    return static_cast<CPerspectiveCamera*>(Camera);
    }

    [[nodiscard]] PRegistry* GetRegistry() const
    {
	    return Registry;
    }

    void OnImGuiRender();

private:
    ICamera* Camera;
    PRegistry* Registry;
};