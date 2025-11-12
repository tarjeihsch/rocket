#pragma once

#include "Actor.h"

class RController : public RActor
{
    using Super = RActor;

public:
    static RClass* StaticClass();

    virtual const RClass* GetClass() const override;
};