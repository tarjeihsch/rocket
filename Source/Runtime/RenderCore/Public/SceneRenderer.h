#pragma once
#include "Camera.h"
#include "Renderer.h"
#include "Transform.h"

#include <vector>

class CStaticMesh;
class CScene;

struct RRenderProxy
{
    CStaticMesh* StaticMesh;

    FTransform* Transform;
};

struct RRenderCamera
{
    ICamera* Camera;
};

class RSceneRenderer : public IRenderer
{
public:
    virtual void Init() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Shutdown() override;

    void RegisterProxy(CStaticMesh* StaticMesh, FTransform* Transform);

    void SetCamera(ICamera* InCamera);

private:
    std::vector<RRenderProxy> Proxies;

    ICamera* Camera = nullptr;
};