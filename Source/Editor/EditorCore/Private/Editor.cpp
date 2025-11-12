#include "Editor.h"

#include "EditorRenderer.h"
#include "Engine.h"
#include "EngineBase.h"
#include "Module.h"
#include "Viewport.h"
#include "World.h"
#include <StaticMeshComponent.h>
#include "CameraComponent.h"
#include "StaticMesh.h"

REditor::REditor()
    : Mode(EMode::Edit),
    Viewport(nullptr),
    ScriptClientModule(nullptr)
{
}

void REditor::Init()
{
    // Create docking UI, tools, asset database
    Viewport = new RViewport();

    EditorViewportClient = new REditorViewportClient();

    ScriptClientModule = new RModule("");

    EditorWorld = new RWorld();

    // TEST
    for (int Idx = 0; Idx < 10; ++Idx)
    {
		RStaticMesh* StaticMesh = new RStaticMesh("C:/Users/Tarjei/Downloads/Untitled.glb");
		RActor* Actor = EditorWorld->SpawnActor(RActor::StaticClass());
		RCameraComponent* CamComp = Actor->AddComponent<RCameraComponent>();
		RStaticMeshComponent* StaticMeshComponent = Actor->AddComponent<RStaticMeshComponent>();
		StaticMeshComponent->SetStaticMesh(StaticMesh);
    }

    // TEST
}

void REditor::Tick(float DeltaTime)
{
    // Draw Editor UI

    Viewport->Update(EditorViewportClient, DeltaTime);

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
    default:
        break;
    }
}

void REditor::Shutdown()
{
    if (Mode != EMode::Edit)
    {
        ExitPlayMode();
    }

    delete EditorViewportClient;

    delete Viewport;

    delete ScriptClientModule;
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

RWorld* REditor::GetWorld() const
{
    return EditorWorld;
}
