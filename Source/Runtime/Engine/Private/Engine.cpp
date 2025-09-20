#include "Engine.h"

#include "Common/RHI.h"
#include "SceneRenderer.h"
#include "World.h"

void REngine::Start(REngineInitParams& InitParams)
{
    SceneRenderer = new RSceneRenderer();
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

void *REngine::GetProjectLibrary() const
{
    return ProjectLibrary;
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
