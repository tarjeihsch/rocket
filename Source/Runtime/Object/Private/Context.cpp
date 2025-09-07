#include "Context.h"

Context::Context()
{
}

Context::~Context()
{
    Shutdown();
}

void Context::Shutdown()
{
    for (auto& Object : Data)
    {
        delete Object;
    }
}

void Context::AddObject(Object* Object)
{
    Data.push_back(Object);
}

void Context::RemoveObject(Object* Object)
{
    Data.remove(Object);
}
