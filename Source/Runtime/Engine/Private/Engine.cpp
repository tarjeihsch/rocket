#include "Engine.h"

#include "../Public/LocalPlayer.h"
#include "SceneRenderer.h"
#include "Viewport.h"
#include "World.h"
#include "CameraComponent.h"
#include "../../Renderer/Public/PrimitiveManager.h"
#include "StaticMesh.h"
#include "StaticMeshComponent.h"

REngine::REngine()
    : World(nullptr)
{
}

void REngine::Start(REngineInitParams& InitParams)
{
    Game = new RGame();
}

void REngine::Tick(float DeltaTime)
{
    World->Tick(DeltaTime);
}

void REngine::Stop()
{
}

RWorld* REngine::GetWorld() const
{
    return World;
}

RGame *REngine::GetGame() const
{
    return Game;
}

void REngine::LoadWorld(const char *Path)
{
    World = new RWorld();

    // TEST BEGIN //

//     RStaticMesh* StaticMesh = new RStaticMesh("/home/acad/Workspace/Atlas/Content/Monkey.glb");
//     RActor* Actor = GetWorld()->SpawnActor(RActor::StaticClass());
//     RCameraComponent* CamComp = Actor->AddComponent<RCameraComponent>();
//     RStaticMeshComponent* StaticMeshComponent = Actor->AddComponent<RStaticMeshComponent>();
//     StaticMeshComponent->SetStaticMesh(StaticMesh);

    // TEST END //
}

void REngine::UnloadWorld()
{
    delete World;
}
