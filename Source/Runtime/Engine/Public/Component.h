#pragma once

#include "World.h"

#include <cstdint>

class CActor;

enum class EComponentFlags : uint8_t
{
    None = 0,
    Tick = 1
};

struct FComponent
{
    FComponent()
        : bShouldTick(0),
        Owner(nullptr)
    {
    }

    virtual ~FComponent() = default;

    virtual void OnInit() {}
    virtual void OnDestroy() {}
    virtual void Tick(float DeltaTime) {}

    inline bool ShouldTick() const;
    inline CActor* GetOwner() const;

private:
    uint8_t bShouldTick : 1;

    CActor* Owner;
};

inline bool FComponent::ShouldTick() const
{
    return bShouldTick;
}

inline CActor *FComponent::GetOwner() const
{
    return Owner;
}