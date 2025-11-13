#pragma once

#include "PrimitiveComponent.h"

class RStaticMesh;

class RStaticMeshComponent final : public RPrimitiveComponent
{
    using Super = RPrimitiveComponent;

public:
    static RClass* StaticClass();

    virtual void Tick(float DeltaTime);
    virtual const RClass* GetClass() const override;
    virtual void OnPropertyChanged() override;

    void SetStaticMesh(RStaticMesh* NewStaticMesh);
    RStaticMesh* GetStaticMesh() const;

private:
    RStaticMesh* StaticMesh;
};