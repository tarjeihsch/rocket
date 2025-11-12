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

void RActor::PostConstruct()
{
    BeginPlay();
}

RComponent* RActor::AddComponent(RClass *Class)
{
    RComponent* Component = (RComponent*)ConstructObject(Class);
    Components.push_back(Component);
    return Component;
}