#include "World.h"

#include "Actor.h"
#include "AssetManager.h"
#include "JsonAsset.h"
#include "Module.h"
#include "../../Renderer/Public/PrimitiveManager.h"

#ifdef RK_PLATFORM_LINUX
#include <dlfcn.h>
#endif

RActor *RWorld::SpawnActor(RClass *Class)
{
    RActor* Actor = ConstructObject<RActor>(this, nullptr);

    Actors.push_back(Actor);

    Actor->BeginPlay();

    return Actor;
}

RWorld::RWorld()
{
    Init();
}

RWorld::~RWorld()
{
    Cleanup();
}

void RWorld::Init()
{
	TickTaskManager = std::make_unique<RTickTaskManager>();
	PrimitiveProxyManager = std::make_unique<RRenderProxyManager>();
}

void RWorld::Cleanup()
{
	for (auto Actor : Actors)
	{
        DestructObject(Actor);
	}
}

void RWorld::DestroyActor(RActor *P_Actor)
{
    P_Actor->EndPlay();

    for (size_t i = 0; i < Actors.size(); ++i)
    {
        if (Actors[i] == P_Actor)
        {
            std::swap(Actors[i], Actors.back());

            RActor* Actor = Actors.back();

            Actors.pop_back();

            DestructObject(Actor);
            
            break;
        }
    }
}

void RWorld::Tick(float DeltaTime)
{
    TickTaskManager->TickFrame(DeltaTime);
}

std::span<RActor*> RWorld::GetActors()
{
    return Actors;
}
