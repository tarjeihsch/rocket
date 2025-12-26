#pragma once

#include "PlatformTypes.h"
#include "Math/Vector2.h"
#include <memory>

struct FRHITexture;
struct FRHISemaphore;

struct RRHIViewportDesc
{
    ENativePlatformBackend Backend;

    // A process instance of the native window system
    RNativeWindowSystemHandle NativeInstanceHandle;

    // A platform-owned handle identifying the drawable OS window
    RNativeWindowHandle NativeWindowHandle;
};

class RRHIViewport
{
public:
    virtual ~RRHIViewport() = default;
    virtual FRHITexture* GetBackBufferTexture() = 0;
    virtual Vector2i GetExtent() const = 0;
    virtual void Acquire() = 0;
    virtual const std::shared_ptr<FRHISemaphore>& GetImageAvailableSemaphore() const = 0;
    virtual const std::shared_ptr<FRHISemaphore>& GetRenderFinishedSemaphore() const = 0;
};
