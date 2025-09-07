#pragma once

#include "Object.h"
#include <list>

class Context
{
public:
    Context();
    ~Context();

    void Shutdown();

    void AddObject(Object* Object);
    void RemoveObject(Object* Object);

    std::list<Object*> Data;
};
