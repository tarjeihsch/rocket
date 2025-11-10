#pragma once

#include "PrimitiveComponent.h"

class RStaticMesh;

class RStaticMeshComponent final : public RPrimitiveComponent
{
    using Super = RSceneComponent;

public:
    static RClass* StaticClass();

    virtual const RClass* GetClass() const override;

    void SetStaticMesh(RStaticMesh* NewStaticMesh);
    RStaticMesh* GetStaticMesh() const;

protected:
    void OnMeshChanged();

private:
    RStaticMesh* StaticMesh;
};