#pragma once

#include "Math/Vector2.h"
#include "PlatformTypes.h"

struct FRHITextureView;
struct FRHITexture;

struct RRHISwapchainDesc
{
    Vector2i Size;
};

class RRHISwapchain
{
public:
    virtual ~RRHISwapchain() = default;

    virtual FRHITexture* GetBackBufferTexture() const = 0;
    virtual FRHITextureView* GetBackBufferTextureView() const = 0;
};