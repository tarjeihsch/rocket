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

class RSceneRenderer
{
public:
    void Init();
    void Tick(float DeltaTime);

    void RegisterProxy(CStaticMesh* StaticMesh, FTransform* Transform);

    void SetCamera(ICamera* InCamera);

    static RSceneRenderer& Get()
    {
        static RSceneRenderer Instance;
        return Instance;
    }

private:
    std::vector<RRenderProxy> Proxies;

    ICamera* Camera = nullptr;
};