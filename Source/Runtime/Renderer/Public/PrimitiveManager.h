#pragma once

#include "Delegate.h"
#include "glm/mat4x4.hpp"

#include <span>

class RStaticMesh;
class RMaterial;
class RViewport;

struct LSceneProxyGeometry
{
    RStaticMesh* StaticMesh;

    RMaterial* Material;
};

struct LSceneProxyTransform
{
    alignas(16)
    glm::mat4 ModelMatrix;

    alignas(16)
    glm::mat4 InverseModelMatrix;
};

struct LSceneProxy
{
    LSceneProxyGeometry Geometry;

    LSceneProxyTransform Transform;
};

struct LSceneView
{
    alignas(16)
    glm::mat4 ProjectionMatrix;

    alignas(16)
    glm::mat4 ViewMatrix;

    // Main, Shadows, UI
    uint32_t ViewFlags;
};

typedef uint64_t LPrimitiveObjectID;
typedef uint64_t LPrimitiveViewID;

class RRenderProxyManager
{
public:
    TDelegate<> OnCreateProxy;
    TDelegate<> OnDestroyProxy;

    [[nodiscard]] uint64_t CreateProxy();
    void RemoveProxy(uint64_t ProxyID);

    void UpdateProxyTransform(uint64_t SceneObjectID, const LSceneProxyTransform& Transform);
    void UpdateProxyGeometry(uint64_t SceneObjectID, const LSceneProxyGeometry& Geometry);
    std::span<LSceneProxy> GetProxies();

    uint64_t CreateView();
    void UpdateView(uint64_t SceneViewID, const LSceneView& SceneView);
    std::span<LSceneView> GetViews();

    void CreateScene();
    void DestroyScene();

private:
    std::vector<LSceneProxy> Proxies;

    std::vector<LSceneView> Views;
};
