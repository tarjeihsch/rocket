#pragma once

#include "EngineBase.h"

class RSceneRenderer;
class RWorld;

class REngine : public IEngineBase
{
public:
    virtual void Start(REngineInitParams& InitParams) override;
    virtual void Tick(float DeltaTime) override;
    virtual void Stop() override;
    virtual void LoadWorld(const char* Path) override;
    virtual void UnloadWorld() override;
    virtual RWorld* GetWorld() const override;

    RSceneRenderer* GetSceneRenderer() const;

private:
    RSceneRenderer* SceneRenderer;

    RWorld* World;
};