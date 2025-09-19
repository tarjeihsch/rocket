#pragma once
#include <cfloat>
#include <cstdint>

enum class ERHIFilterMode : uint8_t
{
    Nearest,   // Point sampling
    Linear     // Linear interpolation
};

enum class ERHIAddressMode : uint8_t
{
    Repeat,          // Wrap around (tile)
    MirroredRepeat,  // Mirror every other tile
    ClampToEdge,     // Clamp to last texel
    ClampToBorder    // Use specified border color
};

enum class ERHIBorderColor : uint8_t
{
    FloatTransparentBlack,
    IntTransparentBlack,
    FloatOpaqueBlack,
    IntOpaqueBlack,
    FloatOpaqueWhite,
    IntOpaqueWhite
};

enum class ERHICompareOp : uint8_t
{
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
};

struct RRHISamplerDesc
{
    // Filtering
    ERHIFilterMode MinFilter;   // Nearest / Linear
    ERHIFilterMode MagFilter;   // Nearest / Linear
    ERHIFilterMode MipFilter;   // Nearest / Linear

    // Addressing (wrap mode)
    ERHIAddressMode AddressU;   // Repeat / Clamp / Mirror / Border
    ERHIAddressMode AddressV;
    ERHIAddressMode AddressW;

    // Mipmapping
    float MipLodBias      = 0.0f;
    float MinLod          = 0.0f;
    float MaxLod          = FLT_MAX;

    // Anisotropy
    bool bEnableAnisotropy = false;
    float MaxAnisotropy    = 1.0f;

    // Border color (only used if AddressMode = Border)
    ERHIBorderColor BorderColor = ERHIBorderColor::FloatOpaqueWhite;

    // Comparison (useful for shadow samplers)
    bool bEnableComparison = false;
    ERHICompareOp CompareOp = ERHICompareOp::Always;
};

class RRHISampler
{
public:
    virtual ~RRHISampler() = default;

    virtual void* GetHandle() const = 0;
    virtual uint32_t GetBindingIndex() const = 0;
};