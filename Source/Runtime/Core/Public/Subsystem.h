#pragma once

#include <vector>
#include <iostream>
#include <span>

// TODO: Use macros SUBSYSTEM_INIT_ENTRYPOINT() and SUBSYSTEM_EXIT_ENTRYPOINT() over the current solution

class ISubsystem 
{
public:
    virtual ~ISubsystem() = default;

    virtual void OnInit() {}
    virtual void OnExit() {}
    virtual void OnTick(float DeltaTime) {}
};

struct SSubsystemStaticRegistry 
{
public:
    static SSubsystemStaticRegistry& Get();

    std::span<ISubsystem*> All()
    {
        return Subsystems;
    }

    void AddSubsystem(ISubsystem* Subsystem)
    {
        Subsystems.push_back(Subsystem);
    }

private:
    std::vector<ISubsystem*> Subsystems;
};

#define REGISTER_SUBSYSTEM(ClassName) \
static struct ClassName##AutoRegister \
{ \
    ClassName##AutoRegister() \
    { \
        SSubsystemStaticRegistry::Get().AddSubsystem(new ClassName()); \
    } \
} ClassName##AutoRegisterInstance;
