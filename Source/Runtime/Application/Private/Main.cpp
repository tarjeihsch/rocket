#include "Main.h"

#include "Common/RHI.h"
#include "ConfigFile.h"
#include "Engine.h"
#include "GLFWWindow.h"
#include "Logger.h"
#include "Module.h"
#include "Timer.h"
#include "WindowBase.h"

STimestep Timestep;

int main(int P_Argc, char* P_Argv[])
{
    REngineInitParams EngineInitParams;
    RWindowInitParams WindowInitParams;

    if (P_Argc == 2)
    {
        std::string ProjectDirectory = P_Argv[1];

        FConfigFile ProjectConfiguration(ProjectDirectory + "/Project.ini");
        EngineInitParams.ProjectRoot = ProjectDirectory;
        EngineInitParams.ProjectName = ProjectConfiguration.GetData<std::string>("Project", "Name");
        EngineInitParams.ProjectBuild = ProjectConfiguration.GetData<std::string>("Project", "Path");
        EngineInitParams.ProjectVersion = ProjectConfiguration.GetData<double>("Project", "Version");
    }

    //EngineInitParams.RHIInterfaceType = ERHIInterfaceType::Vulkan;

    WindowInitParams.Title = "Rocket Engine";
    WindowInitParams.Width = 1280;
    WindowInitParams.Height = 720;

    FLogger::Init();

    //GScriptModule = new RModule((EngineInitParams.ProjectRoot + "/" + EngineInitParams.ProjectBuild).c_str());

    GWindow = new RGLFWWindow();
    GWindow->CreateNativeWindow(WindowInitParams);

    SetRHIModule(ERHIInterfaceType::Vulkan);
    GRHI->Init();

    GEngine = new REngine();
    GEngine->Start(EngineInitParams);
    GEngine->LoadWorld("Content/Start.pkg");

    while (!GWindow->ShouldClose())
    {
        Timestep.Reset();
        GWindow->Poll();
        GEngine->Tick(Timestep.GetDeltaTime());
        GRHI->Tick(Timestep.GetDeltaTime());
    }

    GEngine->UnloadWorld();
    GEngine->Stop();
    GRHI->Shutdown();
    GWindow->DestroyNativeWindow();

    delete GEngine;
    GEngine = nullptr;

    delete GRHI;
    GRHI = nullptr;

    delete GWindow;
    GWindow = nullptr;

    return 0;
}
