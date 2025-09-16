#pragma once

#include "SceneComponent.h"

class CStaticMesh;

class RStaticMeshComponent final : public RSceneComponent
{
    using Super = RSceneComponent;

public:
    static RClass* StaticClass();

    RStaticMeshComponent();
    ~RStaticMeshComponent() override;

    virtual void OnInit() override;
    virtual void OnDestroy() override;
    virtual const RClass* GetClass() const override;

    void SetStaticMesh(CStaticMesh* NewStaticMesh);
    CStaticMesh* GetStaticMesh() const;

private:
    CStaticMesh* StaticMesh;
};