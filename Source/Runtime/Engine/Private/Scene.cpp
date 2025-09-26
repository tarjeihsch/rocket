#include "Scene.h"

#include "../../Renderer/Public/Renderer.h"

uint64_t RScene::CreateSceneObject()
{
    Cache.emplace_back();
    return Cache.size() - 1;
}

void RScene::UpdateSceneObjectTransform(uint64_t ObjectID, const glm::vec3& Translation, const glm::vec3& Rotation, const glm::vec3& Scale)
{
    Cache[ObjectID].Translation = Translation;
    Cache[ObjectID].Rotation = Rotation;
    Cache[ObjectID].Scale = Scale;
}

LSceneObject* RScene::GetSceneObject(uint64_t ObjectID)
{
    return &Cache[ObjectID];
}

void RScene::UpdateSceneObjectGeometry(uint64_t ObjectID, RStaticMesh* Mesh)
{
    Cache[ObjectID].Mesh = Mesh;
}

void RScene::RenderScene(float DeltaTime)
{
    Renderer->Render(DeltaTime);
}
