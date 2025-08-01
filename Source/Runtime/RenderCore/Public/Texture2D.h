#pragma once
#include <cstdint>

struct FRHITexture;

struct FTexture2D
{
    FTexture2D(uint32_t Width, uint32_t Height);
    ~FTexture2D();

    inline FRHITexture* GetHandle() const;

private:
    FRHITexture* Handle;
};

inline FRHITexture* FTexture2D::GetHandle() const
{
    return Handle;
}