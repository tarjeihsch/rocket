#include "Component.h"

#include "World.h"

RClass *RComponent::StaticClass()
{
    static RComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RComponent), alignof(RComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RComponent::RComponent()
    : Owner(nullptr)
{
}

const RClass *RComponent::GetClass() const
{
    return StaticClass();
}

void RComponent::PostConstruct()
{
    GetWorld()->GetTickTaskManager()->AddTickFunction<&RComponent::Tick>(ETickGroup::Default, this);
}

void RComponent::Tick(float DeltaTime)
{
}