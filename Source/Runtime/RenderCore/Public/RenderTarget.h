#pragma once

#include <cstdint>

struct FRHITexture;

struct FRenderTarget
{
    FRenderTarget(uint32_t Width, uint32_t Height);
    ~FRenderTarget();

    inline FRHITexture* GetHandle() const;

private:
    FRHITexture* Handle;
};

inline FRHITexture* FRenderTarget::GetHandle() const
{
    return Handle;
}