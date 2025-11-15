#include "ObjectUtils.h"

#include "Assertion.h"
#include "World.h"
#include <cstring>

RObject* ConstructObject(RWorld* World, RObject* Outer, RClass* Class)
{
    if ((Class->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
        // Perform a manual raw byte copy from archetype to pass the correct vtable pointer from archetype
        void* Block = ::operator new(Class->GetSize(), std::align_val_t(Class->GetAlignment()));
        std::memcpy(Block, static_cast<const void*>(Class->GetArchetype()), Class->GetSize());
        RObject* Object = static_cast<RObject*>(Block);
        Object->PostConstruct(World, Outer);
        return Object;
    }
    ensure(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
    return nullptr;
}

void DestructObject(RObject* Object)
{
    if ((Object->GetClass()->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
        ::operator delete(Object, static_cast<std::align_val_t>(Object->GetClass()->GetAlignment()));
        return;
    }
    ensure(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
}
