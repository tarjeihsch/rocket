#pragma once
#include "Camera.h"
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

class FSceneRenderer
{
public:
    void Init();
    void Tick();

    void RegisterProxy(CStaticMesh* StaticMesh, FTransform* Transform);

    void SetCamera(ICamera* InCamera);

    static FSceneRenderer& Get()
    {
        static FSceneRenderer Instance;
        return Instance;
    }

private:
    std::vector<RRenderProxy> Proxies;

    ICamera* Camera = nullptr;
};