#pragma once

#include "Object.h"

#include <cstdint>

class RWorld;

enum class EComponentFlags : uint8_t
{
    None = 0,
    Tick = 1
};

class RComponent : public RObject
{
    using Super = RObject;
    using This = RComponent;

public:
    static RClass* StaticClass();

    RComponent();

    virtual const RClass* GetClass() const override;
    virtual void PostConstruct(RWorld* World, RObject* Owner) override;

    virtual void Tick(float DeltaTime);
};