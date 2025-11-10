#include "Main.h"

#include "Common/RHI.h"
#include "Editor.h"
#include "GLFWWindow.h"
#include "Logger.h"

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
        GWindow->Poll();
        GEditor->Tick(0.0f);
        GRHI->Tick(0.0f);
    }

    GEditor->Shutdown();
    GRHI->Shutdown();
}