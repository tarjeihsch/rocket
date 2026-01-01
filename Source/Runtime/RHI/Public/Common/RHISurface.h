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

    virtual RHITexture* get_back_buffer_texture() = 0;

    virtual void acquire(RHICommandListContext& ctx) = 0;
    virtual void present(RHICommandListContext& ctx) = 0;

    virtual std::shared_ptr<RHISemaphore> GetImageAvailableSemaphore() const = 0;
    virtual std::shared_ptr<RHISemaphore> GetRenderFinishedSemaphore() const = 0;
};
