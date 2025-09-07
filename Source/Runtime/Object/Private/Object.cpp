#include "Object.h"

#include "Context.h"

static Object* NewObject(Context* Context)
{
    Object* Obj = new Object();

    Context->AddObject(Obj);

    return Obj;
}
