#pragma once

#include "Math/Vector2.h"
#include "PlatformTypes.h"

struct FRHITextureView;
struct FRHITexture;

struct RRHISwapchainDesc
{
    ENativePlatformBackend Backend;

    // A process instance of the native window system
    RNativeWindowSystemHandle NativeInstanceHandle;
    
    // A platform-owned handle identifying the drawable OS window
    RNativeWindowHandle NativeWindowHandle;
        
    Vector2i Size;
};

class RRHISwapchain
{
public:
    virtual ~RRHISwapchain() = default;

    virtual bool AcquireNextImageIndex() = 0;
    virtual void Present() = 0;

    virtual FRHITexture* GetBackBufferTexture() const = 0;
    virtual FRHITextureView* GetBackBufferTextureView() const = 0;
};