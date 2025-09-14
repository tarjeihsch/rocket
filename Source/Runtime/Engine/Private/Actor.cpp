#include "Actor.h"

#include "Component.h"

void CActor::Tick(float DeltaTime)
{
    for (auto Component : M_Components)
    {
        if (Component->ShouldTick())
        {
            Component->Tick(DeltaTime);
        }
    }
}

void CActor::AddComponent(FComponent *Component)
{
    M_Components.push_back(Component);
}

void CActor::RemoveComponent(FComponent* P_Component)
{
    for (uint32_t Index = 0; Index < M_Components.size(); ++Index)
    {
        if (M_Components[Index] == P_Component)
        {
            M_Components[Index] = M_Components.back();

            M_Components.pop_back();

            break;
        }
    }
}

