#include "Engine.h"

#include "SceneRenderer.h"
#include "World.h"

void REngine::Start(REngineInitParams& InitParams)
{
    SceneRenderer = new RSceneRenderer();

    SceneRenderer->Init();
}

void REngine::Tick(float DeltaTime)
{
    World->Tick(DeltaTime);

    SceneRenderer->Tick(DeltaTime);
}

void REngine::Stop()
{
    delete SceneRenderer;
}

RWorld *REngine::GetWorld() const
{
    return World;
}

RSceneRenderer *REngine::GetSceneRenderer() const
{
    return SceneRenderer;
}

void REngine::LoadWorld(const char *Path)
{
    World = new RWorld();

    World->Initialize();
}

void REngine::UnloadWorld()
{
    World->Deinitialize();

    delete World;
}
