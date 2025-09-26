#pragma once
#include "glm/vec3.hpp"

class RActor;
class RStaticMesh;
class IRendererInterface;

// A lightweight proxy representing a visible scene object in world space
struct LSceneObject
{
    glm::vec3 Translation;

    glm::vec3 Rotation;

    glm::vec3 Scale;

    RStaticMesh* Mesh;
};

struct LSceneView
{

};

class RScene
{
public:
    uint64_t CreateSceneObject();
    void UpdateSceneObjectTransform(uint64_t ObjectID, const glm::vec3& Translation, const glm::vec3& Rotation, const glm::vec3& Scale);
    void UpdateSceneObjectGeometry(uint64_t ObjectID, RStaticMesh* Mesh);
    LSceneObject* GetSceneObject(uint64_t ObjectID);

    void RenderScene(float DeltaTime);

private:
    IRendererInterface* Renderer;

    std::vector<LSceneObject> Cache;
};
