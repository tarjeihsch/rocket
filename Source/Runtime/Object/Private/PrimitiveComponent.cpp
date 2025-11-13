#include "PrimitiveComponent.h"

#include "EngineBase.h"
#include "PrimitiveManager.h"
#include "World.h"

RClass* RPrimitiveComponent::StaticClass()
{
    static RPrimitiveComponent Archetype;

    static RClass Class { Super::StaticClass(), &Archetype, sizeof(RPrimitiveComponent), alignof(RPrimitiveComponent), C_ClassDefault | C_Memory_Allocator_Heap };

    return &Class;
}

RPrimitiveComponent::RPrimitiveComponent()
    : bDirty(0), bVisible(0)
{
}

const RClass* RPrimitiveComponent::GetClass() const
{
    return StaticClass();
}

void RPrimitiveComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDirty)
    {
        OnMarkedDirty();
    }
}

void RPrimitiveComponent::SetVisibility(bool bVisibility)
{
    // TODO: Use a free list
    if (bVisibility)
    {
        ProxyHandle = GetWorld()->GetPrimitiveProxyManager()->CreateProxy();
    }
    else
    {
        GetWorld()->GetPrimitiveProxyManager()->RemoveProxy(ProxyHandle);
    }

    bVisible = bVisibility;
}

void RPrimitiveComponent::OnMarkedDirty()
{
    // Reset dirty flag
    bDirty = false;
}