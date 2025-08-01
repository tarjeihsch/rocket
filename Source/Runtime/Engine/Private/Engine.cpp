#include <Subsystem.h>
#include <dlfcn.h>
#include "Engine.h"

#include "AssetManager.h"
#include "Common/RHI.h"
#include "Logger.h"
#include "Scene.h"
#include "SceneRenderer.h"
#include "Window.h"

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

    mAssetManager = new FAssetManager();

    SceneRenderer = new FSceneRenderer();

#if RK_PLATFORM_WINDOWS
    HMODULE GameLib = LoadLibraryA("Game.dll");
#endif

#if RK_PLATFORM_LINUX
    void* GameLib = dlopen("./libGame.so", RTLD_NOW);
#endif

    for (ISubsystem* Subsystem : SSubsystemStaticRegistry::Get().All())
    {
        Subsystem->OnInit();
    }
}

void CEngine::Run()
{
    while (!IWindow::Get()->ShouldClose())
    {
	IWindow::Get()->Poll();
        SceneRenderer->Tick(Scene);
        GRHI->EndFrame();
    }
}

void CEngine::Stop()
{
    GRHI->Shutdown();
	Scene->Cleanup();
	IWindow::Get()->DestroyNativeWindow();

	delete Scene;
    delete mAssetManager;

	GEngine = nullptr;
}
