#pragma once

#include "Context.h"

template<typename T, typename... TArgs>
static T* NewObject(Context* PContext, TArgs... Args)
{
    static_assert(std::is_base_of<Object, T>::value, "T must derive from Object");

    T* Obj = new T(std::forward<TArgs>(Args)...);

    PContext->AddObject(Obj);

    return Obj;
}