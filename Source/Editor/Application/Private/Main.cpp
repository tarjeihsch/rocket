#include "Main.h"

#include "Common/RHI.h"
#include "EditorEngine.h"
#include "FrameGraph.h"
#include "GLFWWindow.h"
#include "Logger.h"
#include <Timer.h>

STimestep Timestep;

int main(int Argc, char** Argv)
{
    FLogger::Init();

    REngineInitParams EngineInitParams;
    RWindowInitParams WindowInitParams;

    WindowInitParams.Title = "Rocket Engine";
    WindowInitParams.Width = 1280;
    WindowInitParams.Height = 720;

    GWindow = new RGLFWWindow();
    GWindow->CreateNativeWindow(WindowInitParams);

    SetRHIModule(ERHIInterfaceType::Vulkan);
    GRHI->Init();

    GEngine = new REditorEngine();
    GEngine->Start(EngineInitParams);

    while (!GWindow->ShouldClose())
    {
        Timestep.Reset();
        GWindow->Poll();
        GEngine->Tick(Timestep.GetDeltaTime());
        GRHI->Tick(Timestep.GetDeltaTime());
    }

    GEngine->Stop();
    GRHI->Shutdown();
}