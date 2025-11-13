#pragma once

#include "PrimitiveComponent.h"

class RStaticMesh;

class RStaticMeshComponent final : public RPrimitiveComponent
{
    using Super = RPrimitiveComponent;

public:
    static RClass* StaticClass();

    virtual const RClass* GetClass() const override;

    void SetStaticMesh(RStaticMesh* NewStaticMesh);
    RStaticMesh* GetStaticMesh() const;

protected:
    virtual void OnMarkedDirty() override;

private:
    RStaticMesh* StaticMesh;
};