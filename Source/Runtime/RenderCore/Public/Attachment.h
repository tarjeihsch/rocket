#pragma once

#include <cstdint>

#include "Common/RHITexture.h"

struct FRHITexture;

class FAttachment
{
public:
    FAttachment(uint32_t Width, uint32_t Height, ERHITextureFormat Format, ERHITextureUsage Usage);
    ~FAttachment();

    inline FRHITexture* GetHandle() const;

private:
    FRHITexture* Handle;
};

inline FRHITexture* FAttachment::GetHandle() const
{
    return Handle;
}