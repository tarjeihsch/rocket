#include "Engine.h"

#include "../Public/LocalPlayer.h"
#include "SceneRenderer.h"
#include "Viewport.h"
#include "World.h"
#include "CameraComponent.h"
#include "Scene.h"

REngine::REngine()
    : Scene(nullptr),
    World(nullptr)
{
}

void REngine::Start(REngineInitParams& InitParams)
{
    Game = new RGame();

    Scene = new RScene();
}

void REngine::Tick(float DeltaTime)
{
    World->Tick(DeltaTime);

    Scene->RenderScene(DeltaTime);
}

void REngine::Stop()
{
}

RWorld* REngine::GetWorld() const
{
    return World;
}

RScene* REngine::GetScene() const
{
    return Scene;
}

RGame *REngine::GetGame() const
{
    return Game;
}

void REngine::LoadWorld(const char *Path)
{
    World = new RWorld();

    World->Initialize();

    // TEST BEGIN //

    RActor* Actor = new RActor();
    RCameraComponent* CamComp = Actor->AddComponent<RCameraComponent>();

    RLocalPlayer* LocalPlayer = new RLocalPlayer();
    Game->LocalPlayers.push_back(LocalPlayer);
    LocalPlayer->GetCameraManager()->CameraComponent = CamComp;

    // TEST END //
}

void REngine::UnloadWorld()
{
    World->Deinitialize();

    delete World;
}
