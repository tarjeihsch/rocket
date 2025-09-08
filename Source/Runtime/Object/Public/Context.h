#pragma once

#include "Object.h"
#include <list>

class Context
{
public:
    Context();
    ~Context();

    void Shutdown();

    void AddObject(CObject* Object);
    void RemoveObject(CObject* Object);

    std::list<CObject*> Data;
};
