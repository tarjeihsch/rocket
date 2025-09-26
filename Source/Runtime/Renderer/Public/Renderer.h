#pragma once

#include <span>

struct LSceneObject;
class RViewport;

class IRendererInterface
{
public:
    virtual ~IRendererInterface() = default;

    virtual void Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Render(std::span<LSceneObject> View) = 0;
};