#include "World.h"

#include "Actor.h"

void CWorld::Tick(float DeltaTime)
{
    for (CActor* Actor : Actors)
    {
        Actor->Tick(DeltaTime);
    }
}
