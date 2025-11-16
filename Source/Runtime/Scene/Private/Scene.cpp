#include "Scene.h"

#include "Assertion.h"

uint64_t RScene::CreateProxy()
{
    Proxies.emplace_back();
    return Proxies.size() - 1;
}

void RScene::RemoveProxy(uint64_t ProxyID)
{
}

void RScene::UpdateProxyTransform(uint64_t SceneObjectID, const LSceneProxyTransform& Transform)
{
    ensure(SceneObjectID < Proxies.size());

    Proxies[SceneObjectID].Transform = Transform;
}

void RScene::UpdateProxyGeometry(uint64_t SceneObjectID, const LSceneProxyGeometry &Geometry)
{
    ensure(SceneObjectID < Proxies.size());

    Proxies[SceneObjectID].Geometry = Geometry;
}

std::span<LSceneActor> RScene::GetProxies()
{
    return std::span<LSceneActor>(Proxies);
}

uint64_t RScene::CreateView()
{
    Views.emplace_back();
    return Views.size() - 1;
}

void RScene::UpdateView(uint64_t SceneViewID, const LSceneCamera& SceneView)
{
    ensure(SceneViewID < Views.size());

    Views[SceneViewID] = SceneView;
}

std::span<LSceneCamera> RScene::GetViews()
{
    return std::span<LSceneCamera>(Views);
}
