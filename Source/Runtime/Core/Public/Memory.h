#pragma once

#include <atomic>
#include <list>
#include <type_traits>

#include "Assert.h"

class FRefCountedObject
{
public:
    void AddRef() const
    {
        ++RefCount;
    }

    void Release() const
    {
        if (--RefCount == 0)
        {
            delete this;
        }
    }

protected:
    virtual ~FRefCountedObject() = default;

private:
    mutable std::atomic<int32_t> RefCount = { 1 };
};

template<typename T>
class TRefCountedPtr
{
    //static_assert(std::is_base_of<FRefCountedObject, T>, "Object is not derived of FRefCountedObject");

public:
    ~TRefCountedPtr();

private:
    T* Pointer;
};

template <typename T>
TRefCountedPtr<T>::~TRefCountedPtr()
{
    RK_ASSERT(Pointer != nullptr);

    delete Pointer;
}

class FLinearAllocator
{
public:
    FLinearAllocator();

    template<typename T, typename... TArgs>
    void Allocate(TArgs... Args);

private:
    uint8_t* Data;
    uint32_t Capacity;
    uint32_t Offset;

    //std::list<FRefCountedResource> List;
};

template <typename T, typename ... TArgs>
void FLinearAllocator::Allocate(TArgs... Args)
{
}
