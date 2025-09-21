#include "Object.h"

const RClass* RObject::GetClass() const
{
    return StaticClass();
}