#pragma once

#include "Assertion.h"
#include "Class.h"

#include <cstring>
#include <vector>

#define PURE_VIRTUAL() { }

class RWorld;

/*
 * Each RObject carries its own integrated RClass metadata layer. This results in boilerplate code that is necessary to perform
 * statical reflection-based lookup and comparisons.
 *
 * Heap-allocated data initialized within the CTOR is not guaranteed to be deep-copied or unique.
 * Use CTOR to initialize trivial data types or structures that are to be shared between instances.
 *
 * Trivially copyable value types such as 'int' and 'float' have copy semantics that work by value.
 * Other complex data types should do their initialization in PostConstruct(). Copy CTOR's are disabled by design.
 *
 */
class RObject
{
public:
    RObject();
    RObject(const RObject&) = delete;
    RObject& operator=(const RObject&) = delete;
    RObject(RObject&&) = delete;
    RObject& operator=(RObject&&) = delete;

    virtual ~RObject() = default;

    static RClass* StaticClass()
    {
        static RObject ClassDefaultObject;

        static RClass Class { "RObject", nullptr, &ClassDefaultObject, sizeof(RObject), alignof(RObject), C_None };

        return &Class;
    }

    virtual const RClass* GetClass() const;

    // Runs a deep copy on pointer/heap-owning fields
    virtual void PostConstruct(class RWorld* InWorld, class RObject* InOuter);
    virtual void Load(const std::vector<uint8_t>& Data) PURE_VIRTUAL()
    virtual void Save(std::vector<uint8_t>& Data) PURE_VIRTUAL()
    virtual void OnPropertyChanged() PURE_VIRTUAL()

    RWorld* GetWorld() const;
    RObject* GetOuter() const;
    uint64_t GetUUID() const;

protected:
    static_assert(DoesImplementStaticClass<RObject>, "");

protected:
    RWorld* World;

    RObject* Outer;

    uint64_t UUID;
};

inline RObject* ConstructObject(RWorld* World, RObject* Owner, RClass* Class)
{
    if ((Class->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
        // Perform a manual raw byte copy from archetype to pass the correct vtable pointer from archetype
        void* Block = ::operator new(Class->GetSize(), std::align_val_t(Class->GetAlignment()));
		std::memcpy(Block, static_cast<const void*>(Class->GetArchetype()), Class->GetSize());
        RObject* NewObj = static_cast<RObject*>(Block);
        NewObj->PostConstruct(World, Owner);
        return NewObj;
    }
    ensure(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
    return nullptr;
}

inline void DestructObject(RObject* Object)
{
    if ((Object->GetClass()->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
		::operator delete(Object, static_cast<std::align_val_t>(Object->GetClass()->GetAlignment()));
        return;
    }
    ensure(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
}

// Helper function for ConstructObject() when type is known at compile-time
template<typename T>
T* ConstructObject(RWorld* World, RObject* Owner)
{
    if ((T::StaticClass()->GetFlags() & C_Memory_Allocator_Heap) == C_Memory_Allocator_Heap)
    {
        // Perform a manual raw byte copy from archetype to pass the correct vtable pointer from archetype
        void* Block = ::operator new(T::StaticClass()->GetSize(), std::align_val_t(T::StaticClass()->GetAlignment()));
        std::memcpy(Block, static_cast<const void*>(T::StaticClass()->GetArchetype()), T::StaticClass()->GetSize());
        T* NewObj = static_cast<T*>(Block);
        NewObj->PostConstruct(World, Owner);
        return NewObj;
    }
    ensure(false, "Incorrect usage of allocator strategy. Explicit control over allocator strategy is required.");
    return nullptr;
}

template <typename T>
struct TObjectPtr
{
    TObjectPtr() : Handle(nullptr) {}
    explicit TObjectPtr(T* InHandle) : Handle(InHandle) {}

    TObjectPtr(const TObjectPtr&) = delete;
    TObjectPtr& operator=(const TObjectPtr&) = delete;

    TObjectPtr(TObjectPtr&& Other) noexcept
        : Handle(Other.Handle)
    {
        Other.Handle = nullptr;
    }

    TObjectPtr& operator=(TObjectPtr&& Other) noexcept
    {
        if (this != &Other)
        {
            DestructObject(Handle);
            Handle = Other.Handle;
            Other.Handle = nullptr;
        }
        return *this;
    }

    TObjectPtr& operator=(T* InHandle)
    {
        if (Handle && Handle != InHandle)
        {
            DestructObject(Handle);
            Handle = InHandle;
        }
        else
        {
            Handle = InHandle;
        }
        return *this;
    }

    ~TObjectPtr()
    {
        if (Handle)
        {
            DestructObject(Handle);
        }
    }

    T* Get() const { return reinterpret_cast<T*>(Handle); }
    T* operator->() const { return reinterpret_cast<T*>(Handle); }
    T& operator*() const { return *reinterpret_cast<T*>(Handle); }
    operator T*() const { return reinterpret_cast<T*>(Handle); }
    explicit operator bool() const { return Handle != nullptr; }

private:
    RObject* Handle;
};

#define RCLASS()
#define RFUNCTION()
#define RFIELD()
