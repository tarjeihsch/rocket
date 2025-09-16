#pragma once

#include "Object.h"

#include <cstdint>

class RActor;

enum class EComponentFlags : uint8_t
{
    None = 0,
    Tick = 1
};

class RComponent : public RObject
{
    using Super = RObject;

public:
    static RClass* StaticClass();

    RComponent();

    virtual const RClass* GetClass() const override;

    virtual void OnInit() {}
    virtual void OnDestroy() {}
    virtual void Tick(float DeltaTime) {}

    inline bool ShouldTick() const;
    inline RActor* GetOwner() const;

private:
    uint8_t bShouldTick : 1;

    RActor* Owner;
};

inline bool RComponent::ShouldTick() const
{
    return bShouldTick;
}

inline RActor *RComponent::GetOwner() const
{
    return Owner;
}