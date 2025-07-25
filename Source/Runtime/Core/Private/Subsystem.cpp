#include "Subsystem.h"

SSubsystemStaticRegistry &SSubsystemStaticRegistry::Get()
{
    static SSubsystemStaticRegistry StaticRegistry;
    return StaticRegistry;
}