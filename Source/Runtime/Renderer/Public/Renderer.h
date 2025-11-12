#pragma once

#include "glm/vec3.hpp"
#include "glm/matrix.hpp"

#include <vector>

struct FFrameGraphBuilder;
class RMaterial;
class RPrimitive;

struct RTransform
{
    glm::vec3 Location;
    glm::vec3 Rotation;
    glm::vec3 Scale;
};

struct LDrawItem
{
    RPrimitive* Primitive;
    RTransform* Transform;
};

struct LDrawView
{
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
};

class RSceneRenderer final
{
public:
    void AddItem(RPrimitive* Primitive, RMaterial* Material, RTransform* Transform);
    void AddView();

    void Render();
    //void Render(RWorld* World, RCamera* Camera);

protected:
    void Render_Opaque_Internal(FFrameGraphBuilder& Builder);

private:
    std::vector<LDrawItem> ItemData;
    std::vector<LDrawView> ViewData;
};

extern RSceneRenderer* GRenderer;

#define RENDERER_ENQUEUE_COMMAND(x) ...
#define RENDERER_ENQUEUE_COMMAND_BATCH(x) ...
