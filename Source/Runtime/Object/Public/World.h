#pragma once

#include "Tickable.h"

#include <Actor.h>
#include <vector>
#include <span>

// TODO: Use an ActorProxy for cache-friendly iteration
// Store a pointer to the real Actor object for cleanup++

class RRenderProxyManager;
class ITickable;

class RWorld final
{
public:
    RWorld();
    ~RWorld();

    void DestroyActor(RActor* Actor);

    template<typename TClass>
    TClass* SpawnActor() const;
    RActor* SpawnActor(RClass* Class);

    void Tick(float DeltaTime);

    void Initialize();
    void Deinitialize();

    std::span<RActor*> GetActors();

    inline RTickTaskManager* GetTickTaskManager() const;
    inline RRenderProxyManager* GetPrimitiveProxyManager() const;

private:
    std::vector<RActor*> Actors;

    RTickTaskManager* TickTaskManager;

    RRenderProxyManager* PrimitiveProxyManager;
};

template <typename TActor>
TActor* RWorld::SpawnActor() const
{
    static_assert(std::derived_from<TActor, RActor>, "TClass must be derived from FActor");

    return static_cast<TActor*>(SpawnActor(TActor::StaticClass()));
}

inline RTickTaskManager *RWorld::GetTickTaskManager() const
{
    return TickTaskManager;
}

inline RRenderProxyManager* RWorld::GetPrimitiveProxyManager() const
{
    return PrimitiveProxyManager;
}