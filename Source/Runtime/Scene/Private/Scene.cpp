#include "Scene.h"

#include "../../Engine/Public/World.h"

void CScene::Init()
{
    Registry = new PRegistry();
}

void CScene::Cleanup()
{
    delete Registry;
}