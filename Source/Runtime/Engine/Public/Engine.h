#pragma once

#include <cstdint>

#include "Timer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class FSceneRenderer;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1280;
static constexpr uint32_t VIEWPORT_HEIGHT = 720;

struct RApplicationInfo
{
    std::string ProjectName;
    std::string ProjectRootPath;
    std::string ProjectLibraryPath;
    double ProjectVersion;
};

class CEngine
{
public:
    CEngine(RApplicationInfo AppInfo);

    void Start();
    void Run();
    void Stop();

    void* GetProjectLibrary() const;
    inline friend CEngine* GetEngine();

private:
    FSceneRenderer* M_SceneRenderer;

    STimestep Timestep;

    RApplicationInfo AppInfo;

    void* ProjectLibrary;

    static CEngine* GEngine;
};

inline CEngine* GetEngine()
{
    return CEngine::GEngine;
}