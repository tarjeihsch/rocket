#include "Tickable.h"

#include <span>

void RTickTaskManager::TickFrame(float DeltaTime)
{
    for (std::span<RTickDelegate> Group : TickGroups)
    {
        for (const RTickDelegate& Entry : Group)
        {
            Entry.Execute(DeltaTime);
        }
    }
}