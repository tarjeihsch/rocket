#include "Class.h"

RClass::RClass(const char* InClassName, RClass* P_ParentClass, RObject* P_Archetype, size_t P_Size, size_t P_Alignment, uint32_t P_Flags, RField* InFields, size_t InFieldCount)
	: ClassName(InClassName), SuperClass(P_ParentClass), Archetype(P_Archetype), Fields(InFields), FieldCount(InFieldCount), Size(P_Size), Alignment(P_Alignment), Flags(P_Flags)
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