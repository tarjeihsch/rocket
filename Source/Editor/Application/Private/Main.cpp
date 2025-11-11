#include "Main.h"

#include "Common/RHI.h"
#include "Editor.h"
#include "GLFWWindow.h"
#include "Logger.h"
#include <Timer.h>

STimestep Timestep;

int main(int Argc, char** Argv)
{
    FLogger::Init();

    RWindowInitParams WindowInitParams = {};
	WindowInitParams.Width = 1280;
    WindowInitParams.Height = 720;
    WindowInitParams.Title = "Rocket Editor";

    GWindow = new RGLFWWindow();
    GWindow->CreateNativeWindow(WindowInitParams);

    SetRHIModule(ERHIInterfaceType::Vulkan);
    GRHI->Init();

    GEditor = new REditor();
    GEditor->Init();

    while (!GWindow->ShouldClose())
    {
        Timestep.Reset();
        GWindow->Poll();
        GEditor->Tick(Timestep.GetDeltaTime());
        GRHI->Tick(Timestep.GetDeltaTime());
    }

    GEditor->Shutdown();
    GRHI->Shutdown();
}