#pragma once

#include "Tickable.h"

#include <Actor.h>
#include <vector>

// TODO: Use an ActorProxy for cache-friendly iteration
// Store a pointer to the real Actor object for cleanup++

class ITickable;

class CWorld final
{
public:
    RActor* SpawnActor(RClass* Class);
    void DestroyActor(RActor* Actor);

    template<typename TClass>
    TClass* SpawnActor() const;

    void Tick(float DeltaTime);

    void Initialize();
    void Deinitialize();

    static CWorld& Get()
    {
        static CWorld Instance;
        return Instance;
    }

    inline RTickTaskManager* GetTickTaskManager() const;

private:
    std::vector<RActor*> Actors;

    RTickTaskManager* TickTaskManager;
};

inline CWorld* GetWorld()
{
    return &CWorld::Get();
}

template <typename TActor>
TActor* CWorld::SpawnActor() const
{
    static_assert(std::derived_from<TActor, RActor>, "TClass must be derived from FActor");

    return static_cast<TActor*>(SpawnActor(TActor::StaticClass()));
}

inline RTickTaskManager *CWorld::GetTickTaskManager() const
{
    return TickTaskManager;
}
