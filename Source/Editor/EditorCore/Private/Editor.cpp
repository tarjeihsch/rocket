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

RStaticMeshComponent* StaticMeshComponent = nullptr;
RStaticMesh* StaticMesh = nullptr;
void REditor::Init()
{
    // Create docking UI, tools, asset database
    Viewport = new RViewport();

    EditorViewportClient = new REditorViewportClient();

    ScriptClientModule = new RModule("");

    EditorWorld = new RWorld();

	// TEST
	{
		RActor* Act = EditorWorld->SpawnActor(RActor::StaticClass());
		RCameraComponent* CamComp = Act->AddComponent<RCameraComponent>();
        CamComp->SetLocation(glm::vec3(-5.0f, 0.0f, 0.0f));
	}

	{
		StaticMesh = new RStaticMesh("C:/Users/Tarjei/Downloads/Untitled.glb");
		RActor* Actor = EditorWorld->SpawnActor(RActor::StaticClass());
		StaticMeshComponent = Actor->AddComponent<RStaticMeshComponent>();
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
        EditorWorld->Tick(DeltaTime);
		StaticMeshComponent->AddRotation(glm::vec3(1.0f) * DeltaTime * 100.0f);
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

    delete EditorWorld;

    delete EditorViewportClient;

    delete Viewport;

    delete ScriptClientModule;

    delete StaticMesh;
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
