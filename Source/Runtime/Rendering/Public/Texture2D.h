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

enum class TextureState : uint8_t
{
    Undefined,
    SRV,
    UAV,
    RenderTarget,
    DepthStencil,
    CopySrc,
    CopyDst,
    Present
};

struct Texture2D
{
    explicit Texture2D(ETextureType InType, uint32_t InWidth, uint32_t InHeight);
    ~Texture2D();

    inline RHITexture* GetHandle() const;

    void LoadRawTextureData(const uint8_t* Data);

protected:
    RHITexture* Handle;

private:
    ETextureType Type;
    uint32_t Width;
    uint32_t Height;
};

inline RHITexture* Texture2D::GetHandle() const
{
    return Handle;
}