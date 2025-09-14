#pragma once

#include <cstdint>

#include "Timer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

class FSceneRenderer;

static const char* VIEWPORT_NAME = "Rocket Engine";
static constexpr uint32_t VIEWPORT_WIDTH = 1280;
static constexpr uint32_t VIEWPORT_HEIGHT = 720;

class CEngine
{
public:
    void Start();
    void Run();
    void Stop();

    inline friend CEngine* GetEngine();

private:
    FSceneRenderer* M_SceneRenderer;

    STimestep Timestep;

    static CEngine* GEngine;
};

inline CEngine* GetEngine()
{
    return CEngine::GEngine;
}