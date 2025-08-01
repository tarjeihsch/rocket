#pragma once

#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class FSceneRenderer;
class PScene;
class IWindow;
class FAssetManager;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1280;
static constexpr uint32_t VIEWPORT_HEIGHT = 720;

class CEngine
{
public:
    void Start();
    void Run();
    void Stop();

    inline PScene* GetScene();
    inline IWindow* GetWindow();
    inline FAssetManager* GetAssetManager();
    inline friend CEngine* GetEngine();

private:
    PScene* Scene;
    IWindow* Window;
    FAssetManager* mAssetManager;
    FSceneRenderer* SceneRenderer;

    static CEngine* GEngine;
};

inline PScene* CEngine::GetScene()
{
    return Scene;
}

inline IWindow* CEngine::GetWindow()
{
    return Window;
}

FAssetManager *CEngine::GetAssetManager()
{
    return mAssetManager;
}

inline CEngine* GetEngine()
{
    return CEngine::GEngine;
}
