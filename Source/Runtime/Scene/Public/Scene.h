#pragma once 

#include "Camera.h"
#include "Delegate.h"

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

private:
    ICamera* ActiveCamera;
};
