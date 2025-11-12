#include "Primitive.h"

#include "Material.h"

RPrimitive::RPrimitive()
{
}

void RPrimitive::Draw(FRHICommandList &CmdList) const
{
    //Material->Bind();
}

RMaterial * RPrimitive::GetMaterial() const
{
    return Material;
}

void RPrimitive::SetMaterial(RMaterial *InMaterial)
{
    Material = InMaterial;
}