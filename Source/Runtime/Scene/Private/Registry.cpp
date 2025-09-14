#include "Registry.h"

#include "Component_Old.h"
#include "Entity.h"

FEntity PRegistry::CreateEntity()
{
    FEntity Entity = {*this};
    Entity.EntityID = Registry.create();
    return Entity;
}

void PRegistry::DestroyEntity(FEntityID EntityID)
{
    Registry.destroy(EntityID);
}

bool PRegistry::IsValid(FEntityID EntityID) const
{
    return Registry.valid(EntityID);
}
