#pragma once

#include "EngineBase.h"
#include "Game.h"

#include <span>
#include <vector>

class RSceneGraph;
class IRendererSystem;
class RRenderProxyManager;
class RViewport;
class RSceneGraph;
class RWorld;

// TODO: Later down the road, we will use the scene as the context when rendering.
// Renderer->RenderScene(*Scene, EditorCameraView);
// Renderer->RenderScene(*Scene, GameCameraView);

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
    virtual RGame* GetGame() const override;

private:
    RGame* Game;

    RWorld* World;
};