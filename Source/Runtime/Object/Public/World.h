#pragma once

#include "Tickable.h"

#include <Actor.h>
#include <vector>

// TODO: Use an ActorProxy for cache-friendly iteration
// Store a pointer to the real Actor object for cleanup++

class ITickable;

class RWorld final
{
public:
    RActor* SpawnActor(RClass* Class);
    void DestroyActor(RActor* Actor);

    template<typename TClass>
    TClass* SpawnActor() const;

    template<typename TClass>
    std::vector<TClass*> GetActorsOfClass();

    void Tick(float DeltaTime);

    void Initialize();
    void Deinitialize();

    inline RTickTaskManager* GetTickTaskManager() const;

private:
    std::vector<RActor*> Actors;

    RTickTaskManager* TickTaskManager;
};

template <typename TActor>
TActor* RWorld::SpawnActor() const
{
    static_assert(std::derived_from<TActor, RActor>, "TClass must be derived from FActor");

    return static_cast<TActor*>(SpawnActor(TActor::StaticClass()));
}

template <typename TClass>
std::vector<TClass*> RWorld::GetActorsOfClass()
{
    std::vector<TClass*> Actors;
}

inline RTickTaskManager *RWorld::GetTickTaskManager() const
{
    return TickTaskManager;
}
