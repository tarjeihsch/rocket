#pragma once

#include "Assertion.h"

#include <array>
#include <cstdint>
#include <vector>

class RObject;

enum class ETickGroup
{
    Default,

    Count
};

struct RTickDelegate
{
    using RFunction = void(*)(void*, float);
    void* Instance = nullptr;
    RFunction Function = nullptr;

    inline void Execute(float DeltaTime) const
    {
        RK_ASSERT(Function, "RTickDelegate::Function is null");

        Function(Instance, DeltaTime);
    }

    template<typename TObject, auto Method>
    static RTickDelegate Bind(TObject* Object)
    {
        static_assert(std::is_member_function_pointer_v<decltype(Method)>, "Method must be a member function");

        static_assert(std::is_invocable_r_v<void, decltype(Method), TObject&, float>, "Method must be callable as (Object, float) and return void");

        return { Object, &Invoke<TObject, Method> };
    }

private:
    template<typename TObject, auto Method>
    static void Invoke(void* Ptr, float DeltaTime)
    {
        (static_cast<TObject*>(Ptr)->*Method)(DeltaTime);
    }
};

class RTickTaskManager
{
public:
    void TickFrame(float DeltaTime);

    template<auto Method, typename T>
    void AddTickFunction(ETickGroup TickGroup, T* Object);

    template<typename T, auto Method>
    void RemoveTickFunction(ETickGroup TickGroup, T* Object);

private:
    std::array<std::vector<RTickDelegate>, static_cast<uint32_t>(ETickGroup::Count)> TickGroups;
};

template <auto Method, typename T>
void RTickTaskManager::AddTickFunction(ETickGroup TickGroup, T *Object)
{
    std::vector<RTickDelegate>& Group = TickGroups.at(static_cast<uint32_t>(TickGroup));

    RTickDelegate Delegate = RTickDelegate::Bind<T, Method>(Object);

    Group.emplace_back(Delegate);
}

template <typename T, auto Method>
void RTickTaskManager::RemoveTickFunction(ETickGroup TickGroup, T* Object)
{
    std::vector<RTickDelegate>& Group = TickGroups.at(static_cast<uint32_t>(TickGroup));

    for (size_t Index = 0; Index < Group.size(); ++Index)
    {
        if (Group[Index].Instance == Object && Group[Index].Function == Method)
        {
            Group[Index] = Group.back();

            Group.pop_back();

            return;
        }
    }
}
