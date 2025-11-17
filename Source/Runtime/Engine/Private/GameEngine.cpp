#include "GameEngine.h"

#include "Viewport.h"
#include "World.h"

RGameEngine::RGameEngine()
    : World(nullptr)
{
}

void RGameEngine::Start(REngineInitParams& InitParams)
{
    Game = new RGame();
}

void RGameEngine::Tick(float DeltaTime)
{
    World->Tick(DeltaTime);
}

void RGameEngine::Stop()
{
}

RWorld* RGameEngine::GetWorld() const
{
    return World;
}

RGame *RGameEngine::GetGame() const
{
    return Game;
}

RRenderer* RGameEngine::GetRenderer() const
{
    return nullptr;
}

RViewport* RGameEngine::GetViewport() const
{
    return nullptr;
}

void RGameEngine::LoadWorld(const char *Path)
{
    World = new RWorld();
}

void RGameEngine::UnloadWorld()
{
    delete World;
}
