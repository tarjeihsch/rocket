#include "EditorEngine.h"

#include "EditorUI.h"
#include "Renderer.h"
#include "World.h"

REditorEngine::REditorEngine()
    : Mode(EMode::Edit)
{
}

void REditorEngine::Start(REngineInitParams& InitParams)
{
    World = std::make_unique<RWorld>();
    Renderer = std::make_unique<RRenderer>();
    UI = std::make_unique<REditorUI>();
}

void REditorEngine::Tick(float DeltaTime)
{
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

    UI->Update(DeltaTime);
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