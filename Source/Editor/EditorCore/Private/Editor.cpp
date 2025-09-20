#include "Editor.h"

#include "EditorRenderer.h"
#include "Engine.h"
#include "EngineBase.h"
#include "Module.h"

REditor::REditor()
    : Mode(EMode::Edit),
    Renderer(nullptr),
    GameModule(nullptr)
{
}

void REditor::Init()
{
    // Create docking UI, tools, asset database
    Renderer = new REditorRenderer();

    GameModule = new RModule("");
}

void REditor::Tick(float DeltaTime)
{
    // Draw Editor UI

    Renderer->Tick(DeltaTime);

    switch (Mode)
    {
    case EMode::Play:
        GEngine->Tick(DeltaTime);
        break;
    case EMode::Edit:
        // Draw editor preview
        break;
    case EMode::Pause:
        // Paused - draw only overlay
        break;
    case EMode::Step:
        GEngine->Tick(1.0f / 60.0f);
        break;
    }
}

void REditor::Shutdown()
{
    if (Mode != EMode::Edit)
    {
        ExitPlayMode();
    }

    delete Renderer;

    delete GameModule;
}

void REditor::EnterPlayMode()
{
    REngineInitParams EngineInitParams;

    GEngine = new REngine();
    GEngine->Start(EngineInitParams);
    GEngine->LoadWorld("");
}

void REditor::ExitPlayMode()
{
    GEngine->UnloadWorld();
    GEngine->Stop();
    delete GEngine;
    GEngine = nullptr;
}

void REditor::PauseFrame()
{
}

void REditor::StepFrame()
{
}

IEditorBase::EMode REditor::GetMode() const
{
    return Mode;
}