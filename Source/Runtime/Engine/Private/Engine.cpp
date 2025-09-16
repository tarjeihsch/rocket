#include "Engine.h"

#include "Common/RHI.h"
#include "Logger.h"
#include "SceneRenderer.h"
#include "Subsystem.h"
#include "Window.h"
#include "World.h"

#ifdef RK_PLATFORM_LINUX
#include <dlfcn.h>
#endif

#ifdef RK_PLATFORM_WINDOWS
#include <windows.h>
#endif

CEngine * CEngine::GEngine = nullptr;

ERHIInterfaceType RHIType = ERHIInterfaceType::Vulkan;

CEngine::CEngine(RApplicationInfo InAppInfo)
    : AppInfo(InAppInfo)
{
}

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

#if RK_PLATFORM_WINDOWS
    ProjectLibrary = LoadLibraryA((AppInfo.ProjectRootPath + "/" + AppInfo.ProjectLibraryPath).c_str());
#endif

#if RK_PLATFORM_LINUX
    ProjectLibrary = dlopen((AppInfo.ProjectRootPath + "/" + AppInfo.ProjectLibraryPath).c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif

    for (ISubsystem* Subsystem : SSubsystemStaticRegistry::Get().All())
    {
        Subsystem->OnInit();
    }

    GetWorld()->Initialize();
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

        GetWorld()->Tick(Timestep.GetDeltaTime());

        FSceneRenderer::Get().Tick();
        G_RHI->EndFrame();
    }
}

void CEngine::Stop()
{
    GetWorld()->Deinitialize();
    G_RHI->Shutdown();
    IWindow::Get()->DestroyNativeWindow();
    GEngine = nullptr;
}

void *CEngine::GetProjectLibrary() const
{
    return ProjectLibrary;;
}
