#include "Component.h"

#include "World.h"

RClass *RComponent::StaticClass()
{
    static RComponent Archetype;

    static RClass Class { "RComponent", Super::StaticClass(), &Archetype, sizeof(RComponent), alignof(RComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RComponent::RComponent()
{
}

const RClass *RComponent::GetClass() const
{
    return StaticClass();
}

void RComponent::PostConstruct(RWorld* InWorld, RObject* InOuter)
{
    Super::PostConstruct(InWorld, InOuter);

    GetWorld()->GetTickTaskManager()->AddTickFunction<&RComponent::Tick>(ETickGroup::Default, this);
}

void RComponent::Tick(float DeltaTime)
{
}