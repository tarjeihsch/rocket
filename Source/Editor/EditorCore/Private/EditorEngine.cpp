#include "EditorEngine.h"

#include "Dockspace.h"
#include "Input.h"
#include "KeyCode.h"
#include "Viewport.h"
#include "Renderer.h"
#include "SceneHierarchyPanel.h"
#include "Texture2D.h"
#include "World.h"
#include "Widget.h"
#include "imgui.h"
#include "Common/RHI.h"
#include <MenuBar.h>

REditorEngine::REditorEngine()
    : Mode(EMode::Edit)
{
}

std::unique_ptr<RTexture2D> FontTex2D;

void REditorEngine::Start(REngineInitParams& InitParams)
{
    World = std::make_unique<RWorld>();
    Renderer = std::make_unique<RRenderer>();
    Viewport = std::make_unique<RViewport>();

    ImGui::CreateContext();
    ImGuiIO& IO = ImGui::GetIO();
    IO.DisplaySize = ImVec2(static_cast<float>(1280), static_cast<float>(720));
    IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    unsigned char* Data;
    int Width, Height;
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&Data, &Width, &Height);

    FontTex2D = std::make_unique<RTexture2D>(ETextureType::Linear, Width, Height);
    FontTex2D->LoadRawTextureData(Data);
    IO.Fonts->SetTexID((ImTextureID)FontTex2D->GetHandle());

	Widget = std::make_unique<RDockspace>();
}

void REditorEngine::Tick(float DeltaTime)
{
    Renderer->BeginFrame();

    switch (Mode)
    {
    case EMode::Play:
        World->Tick(DeltaTime);
        break;

    case EMode::Edit:
        World->Tick(DeltaTime);
        break;

    case EMode::Pause:
        // Paused - draw only overlay
        break;

    case EMode::Step:
        World->Tick(1.0f / 60.0f);
        break;

    default:
        break;
    }

    Viewport->Update(DeltaTime);


    // This should be called inside WidgetManager. Something along the lines of GetSubsystem<RWidgetManager>()->BeginFrame()
    ImGuiIO& IO = ImGui::GetIO();
    IO.DisplaySize = ImVec2(GetRHI()->RHIGetViewportExtent()[0], GetRHI()->RHIGetViewportExtent()[1]);
    IO.DeltaTime = DeltaTime;
    IO.MousePos = ImVec2(PInput::GetMouseX(), PInput::GetMouseY());
    IO.MouseDown[0] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_LEFT);
    IO.MouseDown[1] = PInput::GetMouseButtonPressed(RK_MOUSE_BUTTON_RIGHT);

    // TODO: Use a WidgetManager that goes through all added Widgets, not one specific widget.

    ImGui::NewFrame();
	Widget->Update(DeltaTime);

    // Call GetSubsystem<RWidgetManager>()->EndFrame(). This goes through all CmdLists, converts them to our own RPrimitive, 
    // and hands of the RPrimitive with transform and scale to the Scene.
    ImGui::EndFrame();
    
    // This is commented out. Idea is to defer rendering to the Scene subsystem.
    //Widget->Render(DeltaTime);

    // TODO: Use a unified, global 2D/3D scene system.
    // Accepts any primitives (mesh or widget) for rendering. Specify whether to render using a 2D or 3D view based on a flag.
    // Finally, call GetSubsystem<RSceneManager>()->BuildDrawData();

    Viewport->Present();
	Renderer->EndFrame();
}

void REditorEngine::Stop()
{
    if (Mode != EMode::Edit)
    {
        ExitPlayMode();
    }

    World->Cleanup();
}

void REditorEngine::LoadWorld(const char* Path)
{
}

void REditorEngine::UnloadWorld()
{
}

RWorld* REditorEngine::GetWorld() const
{
    return World.get();
}

RGame* REditorEngine::GetGame() const
{
    return nullptr;
}

RRenderer* REditorEngine::GetRenderer() const
{
    return Renderer.get();
}

RViewport* REditorEngine::GetViewport() const
{
    return Viewport.get();
}

void REditorEngine::EnterPlayMode()
{
    //REngineInitParams EngineInitParams;
    //GEngine = new RGameEngine();
    //GEngine->Start(EngineInitParams);
    //GEngine->LoadWorld("");
}

void REditorEngine::ExitPlayMode()
{
    //GEngine->UnloadWorld();
    //GEngine->Stop();
    //delete GEngine;
    //GEngine = nullptr;
}

void REditorEngine::PauseFrame()
{
}

void REditorEngine::StepFrame()
{
}