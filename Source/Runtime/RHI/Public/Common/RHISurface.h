#pragma once

#include "PlatformTypes.h"
#include "Math/Vector2.h"
#include <memory>

struct RHITexture;
struct RHISemaphore;
class RHICommandListContext;

struct RHISurfaceDesc
{
    ENativePlatformBackend Backend;

    // A process instance of the native window system
    RNativeWindowSystemHandle NativeInstanceHandle;

    // A platform-owned handle identifying the drawable OS window
    RNativeWindowHandle NativeWindowHandle;
};

class RHISurface
{
public:
    virtual ~RHISurface() = default;

    virtual void AcquireBackBuffer() = 0;
    virtual void PresentBackBuffer() = 0;
    virtual RHITexture* GetBackBufferTexture() = 0;
    virtual std::shared_ptr<RHISemaphore> GetImageAvailableSemaphore() const = 0;
    virtual std::shared_ptr<RHISemaphore> GetRenderFinishedSemaphore() const = 0;
};
