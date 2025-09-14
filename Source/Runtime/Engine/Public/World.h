#pragma once

#include <vector>

class FComponent;

class CActor
{
    virtual void Tick(float DeltaTime);

private:
    std::vector<FComponent*> M_Components;
};

class CWorld
{
public:
    template<typename TClass>
    TClass* SpawnActor() const;

    template<typename TClass>
    std::vector<TClass*> GetAllActorsOfType() const;

private:
    std::vector<CActor*> Actors;
};

template <typename TActor>
TActor* CWorld::SpawnActor() const
{
    static_assert(std::derived_from<TActor, CActor>, "TClass must be derived from FActor");

    auto Actor = new TActor();

    Actors.push_back(Actor);

    return Actor;
}

template <typename TActor>
std::vector<TActor*> CWorld::GetAllActorsOfType() const
{
    static_assert(std::derived_from<TActor, CActor>, "TClass must be derived from FActor");

    std::vector<TActor*> Actors;

    for (auto Actor : Actors)
    {
        if (dynamic_cast<TActor*>(Actor) != nullptr)
        {
            Actors.push_back(Actor);
        }
    }
    return Actors;
}