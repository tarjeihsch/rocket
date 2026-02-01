#pragma once

#include <cstdint>

enum class RHITextureLayout : uint8_t
{
    None,
    General,
    ColorAttachment,
    DepthStencil,
    DepthReadOnly,
    ShaderReadOnly,
    TransferSrc,
    TransferDst,
    Present
};

enum class RHITextureFormat
{
    Invalid = 0,
    B8G8R8A8_SRGB,
    R8G8B8A8_UNORM,
    R8G8B8A8_SRGB,
    D32_SFLOAT_S8_UINT,
    D32_SFLOAT,
};

enum class RHITextureAspect
{
    None = 0,
    Color = 1 << 0,
    Depth = 1 << 1,
    Stencil = 1 << 2,
    DepthStencil = Depth | Stencil
};

enum class RHIResourceLifetime : uint8_t
{
    Unknown = 0,

    // Long-lived object
    Persistent,

    // Short-lived object
    Transient,

    // Memory will be managed externally
    Unmanaged,
};

enum class RHIResourceMemory
{
    Unknown = 0,

    // GPU-only memory; fastest access by the GPU; CPU can't map it
    DeviceLocal,

    // GPU-memory; CPU can map this memory - slower access
    HostVisible
};