#include "Object.h"
#include "Assertion.h"
#include "UUID64.h"

RObject::RObject()
    : World(nullptr), 
    Outer(nullptr),
    UUID(UINT64_MAX)
{
}

const RClass* RObject::GetClass() const
{
    return StaticClass();
}

void RObject::PostConstruct(class RWorld* InWorld, class RObject* InOuter)
{
    ensure(InWorld);

    World = InWorld;

    Outer = InOuter;

    UUID = RUUID64();
}
RWorld *RObject::GetWorld() const
{
    ensure(World != nullptr);

    return World;
}

RObject *RObject::GetOuter() const
{
    ensure(Outer != nullptr);

    return Outer;
}

uint64_t RObject::GetUUID() const
{
    return UUID;
}
