#pragma once

#include <vector>

// TODO: Use macros SUBSYSTEM_INIT_ENTRYPOINT() and SUBSYSTEM_EXIT_ENTRYPOINT() over the current solution

class ISubsystem 
{
public:
    virtual ~ISubsystem() = default;

    virtual void OnAwake() {}
    virtual void OnStart() {}
    virtual void OnDetach() {}
    virtual void OnUpdate(float DeltaTime) {}
};

struct SSubsystemStaticRegistry 
{
public:
    static SSubsystemStaticRegistry& GetStaticRegistry() 
    {
        static SSubsystemStaticRegistry StaticRegistry;
        return StaticRegistry;
    }

    std::vector<ISubsystem*>& GetSubsystems()
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
        SSubsystemStaticRegistry::GetStaticRegistry().AddSubsystem(new ClassName()); \
    } \
} ClassName##AutoRegisterInstance;