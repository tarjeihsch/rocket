#include "Object.h"

#include "Context.h"

static CObject* NewObject(Context* Context)
{
    CObject* Obj = new CObject();

    Context->AddObject(Obj);

    return Obj;
}
