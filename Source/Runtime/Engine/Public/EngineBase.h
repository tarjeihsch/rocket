#pragma once

#include "Game.h"

#include <cstdint>
#include <span>
#include <string>

class RScene;
class RViewport;
class RWorld;
enum class ERHIInterfaceType : uint8_t;

struct REngineInitParams
{
    // Platform-agnostic window layer handle
    void* PlatformWindowHandle;

    std::string ProjectRoot;

    std::string ProjectBuild;

    std::string ProjectName;

    double ProjectVersion;

    uint32_t WindowWidth;

    uint32_t WindowHeight;

    ERHIInterfaceType RHIInterfaceType;
};

class IEngineBase
{
public:
    virtual ~IEngineBase() = default;

    virtual void Start(REngineInitParams& InitParams) = 0;
    virtual void Tick(float DeltaTime) = 0;
    virtual void Stop() = 0;

    virtual void LoadWorld(const char* Path) = 0;
    virtual void UnloadWorld() = 0;
    virtual RWorld* GetWorld() const = 0;
    virtual RScene* GetScene() const = 0;
    virtual RGame* GetGame() const = 0;
};

extern IEngineBase* GEngine;