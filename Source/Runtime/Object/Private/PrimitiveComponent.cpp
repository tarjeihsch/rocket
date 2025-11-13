#include "PrimitiveComponent.h"

#include "EngineBase.h"
#include "PrimitiveManager.h"
#include "World.h"

RClass* RPrimitiveComponent::StaticClass()
{
    static RPrimitiveComponent Archetype;

    static RClass Class
    {
        "RPrimitiveComponent",
        Super::StaticClass(),
        &Archetype,
        sizeof(RPrimitiveComponent),
        alignof(RPrimitiveComponent),
        C_ClassDefault | C_Memory_Allocator_Heap
    };

    return &Class;
}

RPrimitiveComponent::RPrimitiveComponent()
    : bMeshDirtyFlag(0)
{
}

const RClass* RPrimitiveComponent::GetClass() const
{
    return StaticClass();
}

//void RPrimitiveComponent::SetVisibility(bool bVisibility)
//{
//    // TODO: Use a free list
//    if (bVisibility)
//    {
//        ProxyHandle = GetWorld()->GetSceneProxyManager()->CreateProxy();
//    }
//    else
//    {
//        GetWorld()->GetSceneProxyManager()->RemoveProxy(ProxyHandle);
//    }
//
//    bVisible = bVisibility;
//}