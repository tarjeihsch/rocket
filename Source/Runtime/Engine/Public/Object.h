#pragma once

#include "Assert.h"
#include "Class.h"

#include <vector>

class RObject
{
public:
    virtual ~RObject() = default;

    static RClass* StaticClass()
    {
        static RObject ClassDefaultObject;

        static RClass Class { nullptr, &ClassDefaultObject, sizeof(RObject), alignof(RObject), C_None };

        return &Class;
    }

    virtual const RClass* GetClass() const;

    // Runs a deep copy on pointer/heap-owning fields
    virtual void PostConstruct() {}
    virtual void Load(const std::vector<uint8_t>& Data) {}
    virtual void Save(std::vector<uint8_t>& Data) {}
};

inline RObject* ConstructObject(RClass* Class)
{
    if ((Class->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
        void* Block = ::operator new(Class->GetSize(), std::align_val_t(Class->GetAlignment()));
        RObject* NewObj = ::new (Block) RObject(*Class->GetArchetype());
        NewObj->PostConstruct();
        return NewObj;
    }
    RK_ASSERT(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
}

inline void DestructObject(RObject* Object)
{
    if ((Object->GetClass()->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
        Object->~RObject();
        ::operator delete(Object, Object->GetClass()->GetSize(), static_cast<std::align_val_t>(Object->GetClass()->GetAlignment()));
    }
    RK_ASSERT(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
}

template<typename T>
T* ConstructObject()
{
    if ((T::StaticClass()->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
        void* Block = ::operator new(T::StaticClass()->GetSize(), std::align_val_t(T::StaticClass()->GetAlignment()));
        T* NewObj = ::new (Block) T(*static_cast<const T*>(T::StaticClass()->GetArchetype()));
        NewObj->PostConstruct();
        return NewObj;
    }
    RK_ASSERT(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
}