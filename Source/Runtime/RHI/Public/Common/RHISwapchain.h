#pragma once

struct FRHITextureView;
struct FRHITexture;

class RRHISwapchain
{
public:
    virtual ~RRHISwapchain() = default;

    virtual bool AcquireNextImageIndex() = 0;
    virtual void Present() = 0;

    virtual FRHITexture* GetBackBufferTexture() const = 0;
    virtual FRHITextureView* GetBackBufferTextureView() const = 0;
};