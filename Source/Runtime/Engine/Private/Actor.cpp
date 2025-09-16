#include "Actor.h"

#include "StaticMeshComponent.h"
#include "TransformComponent.h"

RActor::RActor()
    : StaticMeshComponent(nullptr),
    TransformComponent(nullptr)
{
}

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
    StaticMeshComponent = ConstructObject<RStaticMeshComponent>();

    TransformComponent = ConstructObject<RTransformComponent>();
}