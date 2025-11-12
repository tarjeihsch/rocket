#include "Actor.h"

#include "World.h"

RClass *RActor::StaticClass()
{
    static RActor Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RActor), alignof(RActor), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

const RClass *RActor::GetClass() const
{
    return StaticClass();
}

void RActor::PostConstruct(RWorld* InWorld, RObject* InOwner)
{
    Super::PostConstruct(InWorld, InOwner);

    BeginPlay();
}

RComponent* RActor::AddComponent(RClass *Class)
{
    RComponent* Component = (RComponent*)ConstructObject(World, this, Class);
    Components.push_back(Component);
    return Component;
}

std::span<RComponent*> RActor::GetComponents()
{
    return Components;
}
