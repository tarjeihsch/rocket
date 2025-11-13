#include "Object.h"
#include "Assertion.h"

RObject::RObject()
    : World(nullptr), 
    Outer(nullptr)
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
}
