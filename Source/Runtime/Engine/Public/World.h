#pragma once

#include <vector>

class RActor;
class RComponent;

class CWorld final
{
public:
    template<typename TClass>
    TClass* SpawnActor() const;

    void Tick(float DeltaTime);

    static CWorld& Get()
    {
        static CWorld Instance;
        return Instance;
    }

private:
    std::vector<RActor*> Actors;
};

template <typename TActor>
TActor* CWorld::SpawnActor() const
{
    static_assert(std::derived_from<TActor, RActor>, "TClass must be derived from FActor");

    auto Actor = new TActor();

    Actors.push_back(Actor);

    return Actor;
}
