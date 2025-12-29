#pragma once

#include <cstdint>
#include <array>

struct RHITexture;

struct FRenderTarget
{
    FRenderTarget(uint32_t Width, uint32_t Height);
    ~FRenderTarget();

    inline RHITexture* GetHandle() const;

private:
    RHITexture* Handle;
};

inline RHITexture* FRenderTarget::GetHandle() const
{
    return Handle;
}