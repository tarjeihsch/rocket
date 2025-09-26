#pragma once
#include "Player.h"

class RViewport;
class RActor;
class RCameraComponent;

struct LRenderView;

// Responsible for creating a per-frame scene view from the current active camera
struct RPlayerCameraManager
{
    //RActor* CameraActor;
    RCameraComponent* CameraComponent;
    void Target(RActor* NewCameraActor);

    LRenderView BuildRenderView();
};

class RLocalPlayer final
{
public:
    RLocalPlayer();

    RPlayerCameraManager* GetCameraManager() const;
    RPlayer* GetPlayer() const;
    RViewport* GetViewport() const;

private:
    RPlayerCameraManager* CameraManager;

    RPlayer* Player;

    RViewport* Viewport;
};