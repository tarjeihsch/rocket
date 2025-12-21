#pragma once

#include <cstdint>

struct FRHITexture;

enum class ETextureType
{
    // Standard color texture
    SRGB = 0,

    // Linear color texture (UI, albedo)
    Linear = 1
};

struct RTexture2D
{
    explicit RTexture2D(ETextureType InType, uint32_t InWidth, uint32_t InHeight);
    ~RTexture2D();

    inline FRHITexture* GetHandle() const;

    void LoadRawTextureData(const uint8_t* Data);

protected:
    FRHITexture* Handle;

private:
    ETextureType Type;
    uint32_t Width;
    uint32_t Height;
};

inline FRHITexture* RTexture2D::GetHandle() const
{
    return Handle;
}