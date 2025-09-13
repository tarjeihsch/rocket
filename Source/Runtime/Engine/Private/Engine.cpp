#include <Subsystem.h>
#include "Engine.h"

#include "Assert.h"
#include "AssetManager.h"
#include "Common/RHI.h"
#include "Logger.h"
#include "Scene.h"
#include "SceneRenderer.h"
#include "Window.h"
#include "Context.h"

CEngine* CEngine::GEngine = nullptr;

ERHIInterfaceType RHIType = ERHIInterfaceType::Vulkan;

void CEngine::Start()
{
    GEngine = this;

    M_Context = new Context();

    FLogger::Init();

    SWindowSpecification WindowSpecification { VIEWPORT_NAME, VIEWPORT_WIDTH, VIEWPORT_HEIGHT };

    Scene = new PScene();
    mAssetManager = new FAssetManager();
    SceneRenderer = new FSceneRenderer();

    SetRHIModule(ERHIInterfaceType::Vulkan);

    IWindow::Get()->CreateNativeWindow();
    GRHI->Init();
    Scene->Init();

    for (ISubsystem* Subsystem : SSubsystemStaticRegistry::Get().All())
    {
        Subsystem->OnInit();
    }
}

void CEngine::Run()
{
    while (!IWindow::Get()->ShouldClose())
    {
        Timestep.Reset();

	IWindow::Get()->Poll();

        for (ISubsystem* Subsystem : SSubsystemStaticRegistry::Get().All())
        {
            Subsystem->OnTick(Timestep.GetDeltaTime());
        }

        SceneRenderer->Tick(Scene);
        GRHI->EndFrame();
    }
}

void CEngine::Stop()
{
    M_Context->Shutdown();
    GRHI->Shutdown();
    Scene->Cleanup();
    IWindow::Get()->DestroyNativeWindow();

    delete Scene;
    delete mAssetManager;

    GEngine = nullptr;
}
