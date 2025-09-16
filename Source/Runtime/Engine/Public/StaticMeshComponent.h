#pragma once

#include "Component.h"

class CStaticMesh;

class RStaticMeshComponent final : public RComponent
{
    using Super = RComponent;

public:
    static RClass* StaticClass();

    RStaticMeshComponent();

    virtual const RClass* GetClass() const override;

    void SetStaticMesh(CStaticMesh* NewStaticMesh);
    CStaticMesh* GetStaticMesh() const;

private:
    struct RStaticMeshRenderProxy
    {
        CStaticMesh* Handle;
    };

    RStaticMeshRenderProxy RenderProxy;
};