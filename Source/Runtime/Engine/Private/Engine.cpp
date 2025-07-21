#include "Engine.h"

#include "Common/RHI.h"
#include "Window.h"
#include "Logger.h"
#include "Scene.h"

CEngine* CEngine::GEngine = nullptr;

ERHIInterfaceType RHIType = ERHIInterfaceType::Vulkan;

void CEngine::Start()
{
	GEngine = this;

	FLogger::Init();

	SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };

	Scene = new PScene();

	SetRHIModule(ERHIInterfaceType::Vulkan);

    IWindow::Get()->CreateNativeWindow();
    GRHI->Init();
	Scene->Init();
}

void CEngine::Run()
{
	while (!IWindow::Get()->ShouldClose())
	{
		IWindow::Get()->Poll();

        GRHI->Tick(0.0f);
	}
}

void CEngine::Stop()
{
    GRHI->Shutdown();
	Scene->Cleanup();
	IWindow::Get()->DestroyNativeWindow();

	delete Scene;

	GEngine = nullptr;
}
