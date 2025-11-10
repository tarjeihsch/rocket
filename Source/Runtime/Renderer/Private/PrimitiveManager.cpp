#include "../Public/PrimitiveManager.h"

#include "Assert.h"

uint64_t RRenderProxyManager::CreateProxy()
{
    Proxies.emplace_back();
    return Proxies.size() - 1;
}

void RRenderProxyManager::RemoveProxy(uint64_t ProxyID)
{
}

void RRenderProxyManager::UpdateProxyTransform(uint64_t SceneObjectID, const LSceneProxyTransform& Transform)
{
    assert(SceneObjectID < Proxies.size());

    Proxies[SceneObjectID].Transform = Transform;
}

void RRenderProxyManager::UpdateProxyGeometry(uint64_t SceneObjectID, const LSceneProxyGeometry &Geometry)
{
    assert(SceneObjectID < Proxies.size());

    Proxies[SceneObjectID].Geometry = Geometry;
}

std::span<LSceneProxy> RRenderProxyManager::GetProxies()
{
    return std::span<LSceneProxy>(Proxies);
}

uint64_t RRenderProxyManager::CreateView()
{
    Views.emplace_back();
    return Views.size() - 1;
}

void RRenderProxyManager::UpdateView(uint64_t SceneViewID, const LSceneView& SceneView)
{
    assert(SceneViewID < Views.size());

    Views[SceneViewID] = SceneView;
}

std::span<LSceneView> RRenderProxyManager::GetViews()
{
    return std::span<LSceneView>(Views);
}