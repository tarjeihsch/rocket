#include "Engine.h"

#include "Viewport.h"
#include "World.h"

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
}

void REngine::UnloadWorld()
{
    delete World;
}
