#pragma once

#include <cstdint>
#include <memory>

class FRenderTarget;

struct FFrameGraphBuilder;

enum EViewportMode : uint8_t
{
    VT_Game,
    VT_GamePreview,
    VT_MaterialPreview,
    VT_Count
};

class RViewport
{
public:
    RViewport();
    ~RViewport();

    void Update(float DeltaTime);
    void Present();

private:
    void Init();
    void Shutdown();

public:
    std::shared_ptr<FRenderTarget> RenderTarget;
};