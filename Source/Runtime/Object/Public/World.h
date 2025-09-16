#pragma once

#include <Actor.h>
#include <vector>

// TODO: Use an ActorProxy for cache-friendly iteration
// Store a pointer to the real Actor object for cleanup++

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

private:
    std::vector<RActor*> Actors;
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
