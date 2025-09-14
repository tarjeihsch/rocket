#pragma once

#include <cstdint>
#include <string>

class ICamera;

struct IComponent_Old {};

struct FUUIDComponent : IComponent_Old
{
    FUUIDComponent() = default;

    uint64_t UUID;
};

struct FTagComponent : IComponent_Old
{
    FTagComponent() = default;
    FTagComponent(const std::string& InTag) : Tag(InTag) {}

    std::string Tag;
};

struct FCameraComponent : IComponent_Old
{
    FCameraComponent() = default;
    FCameraComponent(ICamera* InCamera) : Camera(InCamera) {}

    ICamera* Camera;
};
