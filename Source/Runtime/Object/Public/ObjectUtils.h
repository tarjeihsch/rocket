#pragma once

class RObject;
class RWorld;
class RClass;

RObject* ConstructObject(RWorld* World, RObject* Outer, RClass* Class);

void DestructObject(RObject* Object);

// Helper function for ConstructObject() when type is known at compile-time
template<typename T>
T* ConstructObject(RWorld* World, RObject* Owner)
{
    return static_cast<T*>(ConstructObject(World, Owner, T::StaticClass()));
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