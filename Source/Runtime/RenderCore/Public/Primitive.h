#pragma once

class FRHICommandList;
class RMaterial;

class RPrimitive
{
public:
    RPrimitive();

    virtual void Draw(FRHICommandList& CmdList) const;

    RMaterial* GetMaterial() const;
    void SetMaterial(RMaterial* InMaterial);

private:
    RMaterial* Material;
};