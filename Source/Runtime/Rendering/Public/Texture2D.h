#pragma once

#include <cstdint>

struct RHITexture;

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

    inline RHITexture* GetHandle() const;

    void LoadRawTextureData(const uint8_t* Data);

protected:
    RHITexture* Handle;

private:
    ETextureType Type;
    uint32_t Width;
    uint32_t Height;
};

inline RHITexture* RTexture2D::GetHandle() const
{
    return Handle;
}