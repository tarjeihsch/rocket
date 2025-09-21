#pragma once

#include "EngineBase.h"

#include "Timer.h"
#include "World.h"

// #define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class RSceneRenderer;

class REngine : public IEngineBase
{
public:
    virtual void Start(REngineInitParams& InitParams) override;
    virtual void Tick(float DeltaTime) override;
    virtual void Stop() override;
    virtual void LoadWorld(const char* Path) override;
    virtual void UnloadWorld() override;
    virtual RWorld* GetWorld() const override;

    void* GetProjectLibrary() const;
    RSceneRenderer* GetSceneRenderer() const;

    inline const STimestep& GetTimestep() const
    {
        return Timestep;
    }

private:
    RSceneRenderer* SceneRenderer;

    RWorld* World;

    void* ProjectLibrary;
};