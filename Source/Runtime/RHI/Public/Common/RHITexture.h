#pragma once

#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>

enum class ERHITextureFlags : uint8_t
{
    None = 0,

    Transient = 1 << 0,

    Unmanaged = 1 << 1,
};

inline ERHITextureFlags operator|(ERHITextureFlags Lhs, ERHITextureFlags Rhs) { return static_cast<ERHITextureFlags>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline ERHITextureFlags operator&(ERHITextureFlags Lhs, ERHITextureFlags Rhs) { return static_cast<ERHITextureFlags>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

enum class ERHITextureUsage : uint8_t
{
    None = 0,

    Present = 1 << 0, // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT (indirectly used for presentation via swapchain)

    Color = 1 << 1, // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT

    Depth = 1 << 2, // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT

    Stencil = 1 << 3, // VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT (used with stencil aspects)

    CopySrc = 1 << 4, // VK_IMAGE_USAGE_TRANSFER_SRC_BIT

    CopyDst = 1 << 5, // VK_IMAGE_USAGE_TRANSFER_DST_BIT

    SRV = 1 << 6, // VK_IMAGE_USAGE_SAMPLED_BIT

    UAV = 1 << 7  // VK_IMAGE_USAGE_STORAGE_BIT
};

inline ERHITextureUsage operator|(ERHITextureUsage Lhs, ERHITextureUsage Rhs) { return static_cast<ERHITextureUsage>(static_cast<uint8_t>(Lhs) | static_cast<uint8_t>(Rhs)); }
inline ERHITextureUsage operator&(ERHITextureUsage Lhs, ERHITextureUsage Rhs) { return static_cast<ERHITextureUsage>(static_cast<uint8_t>(Lhs) & static_cast<uint8_t>(Rhs)); }

struct FRHITextureDesc
{
    //union
    //{
    //    uint32_t Extent[2];
    //} Tex2D;

    //ERHITextureFlags Flags;
    //ERHITextureUsage Usage;

    //static FRHITextureDesc Create2D(uint32_t SizeX, uint32_t SizeY, ERHITextureUsage Usage, ERHITextureFlags InFlags)
    //{
    //    FRHITextureDesc Desc = {};
    //    Desc.Tex2D.Extent[0] = SizeX;
    //    Desc.Tex2D.Extent[1] = SizeY;
    //    Desc.Flags = InFlags;
    //    Desc.Usage = Usage;
    //    return Desc;
    //}

    VkExtent3D Extent;
    VkImageUsageFlags Usage;
    VkFormat Format;
    VkImageLayout Layout;
    ERHITextureFlags Flags;
    uint32_t MipLevels;
    uint32_t ArrayLayers;
    VkSampleCountFlagBits SampleCountFlags;
    VmaVirtualBlock VirtualMemory;

    static FRHITextureDesc Create2D(VkExtent3D Extent, VkFormat Format, VkImageUsageFlags Usage, ERHITextureFlags Flags)
    {
        FRHITextureDesc Desc = {};
        Desc.Extent = Extent;
        Desc.Usage = Usage;
        Desc.Format = Format;
        Desc.Flags = Flags;
        return Desc;
    }
};

struct FRHITextureViewDesc
{
    VkFormat Format;
    VkImageAspectFlags AspectFlags;

    static FRHITextureViewDesc Create2D(VkFormat Format, VkImageAspectFlags AspectFlags)
    {
        FRHITextureViewDesc Desc = {};
        Desc.Format = Format;
        Desc.AspectFlags = AspectFlags;
        return Desc;
    }
};

struct FRHITexture
{
    virtual ~FRHITexture() = default;

    virtual const FRHITextureDesc& RHIGetTextureDesc() const = 0;
};

struct FRHITextureView
{
    virtual ~FRHITextureView() = default;
};