#pragma once

#include <cstdint>

#include "Timer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class FSceneRenderer;
class IWindow;
class FAssetManager;
class CWorld;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1280;
static constexpr uint32_t VIEWPORT_HEIGHT = 720;

class CEngine
{
public:
    void Start();
    void Run();
    void Stop();

    inline IWindow* GetWindow();
    inline FAssetManager* GetAssetManager();
    inline friend CEngine* GetEngine();

private:
    IWindow* Window;
    FAssetManager* M_AssetManager;
    FSceneRenderer* M_SceneRenderer;

    STimestep Timestep;

    static CEngine* GEngine;
};

inline IWindow* CEngine::GetWindow()
{
    return Window;
}

FAssetManager *CEngine::GetAssetManager()
{
    return M_AssetManager;
}

inline CEngine* GetEngine()
{
    return CEngine::GEngine;
}