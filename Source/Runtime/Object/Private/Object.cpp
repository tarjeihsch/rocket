#include "Object.h"
#include "Assertion.h"

RObject::RObject()
    : World(nullptr), 
    Owner(nullptr)
{
}

const RClass* RObject::GetClass() const
{
    return StaticClass();
}

void RObject::PostConstruct(class RWorld* InWorld, class RObject* InOwner)
{
    ensure(InWorld);

    World = InWorld;

    Owner = InOwner;
}
