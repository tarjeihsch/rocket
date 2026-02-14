#pragma once

#include "Common/RHITypes.h"
#include "VulkanLoader.h"

namespace RHI
{
    static VkFormat GetNativeFormat(GPUFormat format)
    {
        switch (format)
        {
            case GPUFormat::Unknown:         return VK_FORMAT_UNDEFINED;

            case GPUFormat::RGBA8_UNORM:     return VK_FORMAT_R8G8B8A8_UNORM;
            case GPUFormat::RGBA8_SRGB:      return VK_FORMAT_R8G8B8A8_SRGB;
            case GPUFormat::BGRA8_UNORM:     return VK_FORMAT_B8G8R8A8_UNORM;
            case GPUFormat::BGRA8_SRGB:      return VK_FORMAT_B8G8R8A8_SRGB;

            case GPUFormat::R16_FLOAT:       return VK_FORMAT_R16_SFLOAT;
            case GPUFormat::RG16_FLOAT:      return VK_FORMAT_R16G16_SFLOAT;
            case GPUFormat::RGBA16_FLOAT:    return VK_FORMAT_R16G16B16A16_SFLOAT;

            case GPUFormat::R32_FLOAT:       return VK_FORMAT_R32_SFLOAT;
            case GPUFormat::RG32_FLOAT:      return VK_FORMAT_R32G32_SFLOAT;
            case GPUFormat::RGB32_FLOAT:     return VK_FORMAT_R32G32B32_SFLOAT;
            case GPUFormat::RGBA32_FLOAT:    return VK_FORMAT_R32G32B32A32_SFLOAT;

            case GPUFormat::RGBA32_UINT:     return VK_FORMAT_R32G32B32A32_UINT;
            case GPUFormat::RGBA32_SINT:     return VK_FORMAT_R32G32B32A32_SINT;
            case GPUFormat::RGB32_UINT:      return VK_FORMAT_R32G32B32_UINT;
            case GPUFormat::RGB32_SINT:      return VK_FORMAT_R32G32B32_SINT;
            case GPUFormat::R32_UINT:        return VK_FORMAT_R32_UINT;

            case GPUFormat::D16_UNORM:       return VK_FORMAT_D16_UNORM;
            case GPUFormat::D32_FLOAT:       return VK_FORMAT_D32_SFLOAT;
            case GPUFormat::D24_UNORM_S8:    return VK_FORMAT_D24_UNORM_S8_UINT;
            case GPUFormat::D32_FLOAT_S8:    return VK_FORMAT_D32_SFLOAT_S8_UINT;
        }

        return VK_FORMAT_UNDEFINED;
    }
}
