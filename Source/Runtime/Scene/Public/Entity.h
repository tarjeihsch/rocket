#pragma once

#include "Scene.h"

class FEntity
{
public:
    FEntity(PRegistry& InRegistry)
        : Registry(InRegistry)
    {
    }

    template<typename TComponent, typename... TArgs>
    TComponent& AddComponent(TArgs&&... Args) const;

    template<typename TComponent>
    TComponent& GetComponent() const;

    template<typename TComponent>
    bool HasComponent() const;

    template<typename TComponent>
    void RemoveComponent();

    [[nodiscard]] FEntityID GetEntityID() const
    {
        return EntityID;
    }

private:
    FEntityID EntityID;
    PRegistry& Registry;

    friend class PRegistry;
};

template<typename TComponent, typename... TArgs>
TComponent& FEntity::AddComponent(TArgs&&... Args) const
{
    TComponent& Component = Registry.AddComponent<TComponent>(EntityID, std::forward<TArgs>(Args)...);
    return Component;
}

template<typename TComponent>
TComponent& FEntity::GetComponent() const
{
    TComponent& Component = Registry.GetComponent<TComponent>(EntityID);
    return Component;
}

template<typename TComponent>
bool FEntity::HasComponent() const
{
    return Registry.HasComponent<TComponent>(EntityID);
}

template<typename TComponent>
void FEntity::RemoveComponent()
{
    Registry.RemoveComponent<TComponent>(EntityID);
}