#pragma once

#include "Math/Vector2.h"
#include "PlatformTypes.h"

struct RHISemaphore;
struct FRHITextureView;
struct RHITexture;

struct RRHISwapchainDesc
{
    Vector2i Size;
};

class RHISwapchain
{
public:
    virtual ~RHISwapchain() = default;
};