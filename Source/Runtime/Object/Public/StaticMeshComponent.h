#pragma once

#include "SceneComponent.h"

class RStaticMesh;

class RStaticMeshComponent final : public RSceneComponent
{
    using Super = RSceneComponent;

public:
    static RClass* StaticClass();

    RStaticMeshComponent();

    virtual const RClass* GetClass() const override;
    virtual void Tick(float DeltaTime) override;

    void SetStaticMesh(RStaticMesh* NewStaticMesh);
    RStaticMesh* GetStaticMesh() const;

protected:
    void OnMeshChanged();

private:
    uint64_t SceneObjectID;

    RStaticMesh* StaticMesh;

    uint8_t bDirty : 1;
};