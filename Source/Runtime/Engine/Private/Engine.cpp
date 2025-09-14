#include <Subsystem.h>
#include "Engine.h"

#include "AssetManager.h"
#include "Common/RHI.h"
#include "Logger.h"
#include "SceneRenderer.h"
#include "Window.h"

CEngine* CEngine::GEngine = nullptr;

ERHIInterfaceType RHIType = ERHIInterfaceType::Vulkan;

void CEngine::Start()
{
    GEngine = this;

    FLogger::Init();

    M_AssetManager = new FAssetManager();
    M_SceneRenderer = new FSceneRenderer();

    SetRHIModule(ERHIInterfaceType::Vulkan);

    IWindow::Get()->CreateNativeWindow();
    G_RHI->Init();

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

        //M_SceneRenderer->Tick(M_Scene);
        G_RHI->EndFrame();
    }
}

void CEngine::Stop()
{
    G_RHI->Shutdown();
    IWindow::Get()->DestroyNativeWindow();

    delete M_AssetManager;

    GEngine = nullptr;
}
