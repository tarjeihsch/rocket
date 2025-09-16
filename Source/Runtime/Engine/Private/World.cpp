#include "World.h"

#include "Actor.h"

void CWorld::Tick(float DeltaTime)
{
    for (RActor* Actor : Actors)
    {
        Actor->Tick(DeltaTime);
    }
}
