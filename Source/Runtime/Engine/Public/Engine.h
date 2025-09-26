#pragma once

#include "EngineBase.h"
#include "Game.h"

#include <span>
#include <vector>

class RScene;
class RViewport;
class RSceneRenderer;
class RWorld;

class REngine : public IEngineBase
{
public:
    REngine();

    virtual void Start(REngineInitParams& InitParams) override;
    virtual void Tick(float DeltaTime) override;
    virtual void Stop() override;
    virtual void LoadWorld(const char* Path) override;
    virtual void UnloadWorld() override;
    virtual RWorld* GetWorld() const override;
    virtual RScene* GetScene() const override;
    virtual RGame* GetGame() const override;

private:
    RWorld* World;
    RGame* Game;
    RScene* Scene;
};