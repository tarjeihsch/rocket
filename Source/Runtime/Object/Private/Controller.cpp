#include "Controller.h"

RClass *RController::StaticClass()
{
    static RController ClassDefaultObject;

    static RClass Class { "RController", Super::StaticClass(), &ClassDefaultObject, sizeof(RController), alignof(RController), C_None };

    return &Class;
}

const RClass* RController::GetClass() const
{
    return StaticClass();
}