#pragma once

#include <cstdint>

#include "Common/RHITexture.h"

struct FRHITexture;

// Read-only texture memory

struct RTexture2D
{
    RTexture2D(uint32_t Width, uint32_t Height);
    ~RTexture2D();

    inline FRHITexture* GetHandle() const;

    void UploadImage(const uint8_t* Data);

private:
    FRHITexture* Handle;
    uint32_t Width;
    uint32_t Height;
};

inline FRHITexture* RTexture2D::GetHandle() const
{
    return Handle;
}