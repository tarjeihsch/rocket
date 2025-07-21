#pragma once 

#include "Camera.h"
#include "Registry.h"

class PScene
{
public:
	void Init()
	{
		Camera = new PCamera();
		Registry = new PRegistry();
	}

	void Cleanup() 
	{
		delete Camera;
		delete Registry;
	}

	[[nodiscard]] PCamera* GetCamera() const
	{
		return Camera;
	}

	[[nodiscard]] PRegistry* GetRegistry() const
	{
		return Registry;
	}

	void OnImGuiRender();

private:
	PCamera* Camera;
	PRegistry* Registry;
};