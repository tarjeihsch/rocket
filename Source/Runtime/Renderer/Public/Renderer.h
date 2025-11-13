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
    RTransform Transform;
};

struct LDrawView
{
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
};

class RSceneGraph final
{
public:
private:
    std::vector<LDrawItem> ItemData;
    std::vector<LDrawView> ViewData;
};

extern RSceneGraph* GRenderer;

#define RENDERER_ENQUEUE_COMMAND(x) ...
#define RENDERER_ENQUEUE_COMMAND_BATCH(x) ...
