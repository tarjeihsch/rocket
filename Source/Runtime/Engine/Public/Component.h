#pragma once

#include <cstdint>

enum class EComponentFlags : uint8_t
{
    None = 0,
    Tick = 1
};

struct FComponent
{
    virtual ~FComponent() = default;

    virtual void OnInit() {}
    virtual void OnDestroy() {}
    virtual void Tick(float DeltaTime) {}

    inline bool ShouldTick() const;

private:
    uint8_t bShouldTick : 1;
};

inline bool FComponent::ShouldTick() const
{
    return bShouldTick;
}