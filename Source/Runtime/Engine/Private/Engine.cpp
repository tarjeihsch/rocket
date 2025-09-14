#include "Engine.h"

#include "Common/RHI.h"
#include "Logger.h"
#include "SceneRenderer.h"
#include "Subsystem.h"
#include "Window.h"

CEngine * CEngine::GEngine = nullptr;

ERHIInterfaceType RHIType = ERHIInterfaceType::Vulkan;

void CEngine::Start()
{
    GEngine = this;

    FLogger::Init();

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
    GEngine = nullptr;
}
