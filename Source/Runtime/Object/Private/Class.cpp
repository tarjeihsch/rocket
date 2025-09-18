#include "Class.h"

RClass::RClass(RClass* P_ParentClass, RObject* P_Archetype, uint64_t P_Size, uint64_t P_Alignment, uint32_t P_Flags)
    : SuperClass(P_ParentClass), Archetype(P_Archetype), Size(P_Size), Alignment(P_Alignment), Flags(P_Flags)
{
}

bool RClass::IsA(const RClass *Other) const
{
    for (const RClass* Class = this; Class; Class = Class->GetSuperClass())
    {
        if (Class == Other)
        {
            return true;
        }
    }
    return false;
}