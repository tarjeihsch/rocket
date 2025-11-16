#pragma once

#include "Delegate.h"
#include "glm/mat4x4.hpp"

#include <span>

class RStaticMesh;
class RMaterial;
class RViewport;

struct LScenePrimitive2D
{
    float Scale[2];

    float Translate[2];
};

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

struct LSceneActor
{
    LSceneProxyGeometry Geometry;

    LSceneProxyTransform Transform;
};

struct LSceneCamera
{
    alignas(16)
    glm::mat4 ProjectionMatrix;

    alignas(16)
    glm::mat4 ViewMatrix;
};

class RScene
{
public:
    [[nodiscard]] uint64_t CreateProxy();
    void RemoveProxy(uint64_t ProxyID);

    void UpdateProxyTransform(uint64_t SceneObjectID, const LSceneProxyTransform& Transform);
    void UpdateProxyGeometry(uint64_t SceneObjectID, const LSceneProxyGeometry& Geometry);
    std::span<LSceneActor> GetProxies();

    [[nodiscard]] uint64_t CreateView();
    void UpdateView(uint64_t SceneViewID, const LSceneCamera& SceneView);
    std::span<LSceneCamera> GetViews();

private:
    std::vector<LSceneActor> Proxies;
    std::vector<LSceneCamera> Views;
};

#define RENDERER_ENQUEUE_COMMAND(x) ...
#define RENDERER_ENQUEUE_COMMAND_BATCH(x) ...