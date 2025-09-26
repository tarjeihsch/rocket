#include "../Public/LocalPlayer.h"

#include "CameraComponent.h"
#include "Viewport.h"

RLocalPlayer::RLocalPlayer()
    : CameraManager(nullptr)
{
    CameraManager = new RPlayerCameraManager();
}

RPlayerCameraManager* RLocalPlayer::GetCameraManager() const
{
    return CameraManager;
}

LRenderView RPlayerCameraManager::BuildRenderView()
{
    //RCameraComponent* CameraComponent = CameraActor->GetComponent<RCameraComponent>();
    if (CameraComponent)
    {
        return CameraComponent->BuildRenderView();
    }
}

//void LPlayerCameraManager::Target(RActor* NewCameraActor)
//{
//    RCameraComponent* CameraComponent = NewCameraActor->GetComponent<RCameraComponent>();
//    if (CameraComponent)
//    {
//        CameraActor = NewCameraActor;
//    }
//}
