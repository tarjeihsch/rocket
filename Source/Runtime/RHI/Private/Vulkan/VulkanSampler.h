#pragma once
#include "Assert.h"
#include "Common/RHISampler.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include <vulkan/vulkan_core.h>

class RVulkanSampler : public RRHISampler
{
public:
    RVulkanSampler(FVulkanDevice& Device, RRHISamplerDesc& Desc);
    ~RVulkanSampler() override;

    virtual void* GetHandle() const override;
    virtual uint32_t GetBindingIndex() const override;

    void Alloc();
    void Free();

    RRHISamplerDesc Desc;
    VkSampler Sampler;

    FVulkanDevice& Device;

    DescriptorHandle M_DescriptorHandle;
};

inline void RVulkanSampler::Alloc()
{
    VkSamplerCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

    // Filtering (map from your ERHIFilterMode)
    info.magFilter = VulkanRHI::ToVkFilter(Desc.MagFilter);
    info.minFilter = VulkanRHI::ToVkFilter(Desc.MinFilter);
    info.mipmapMode = (Desc.MipFilter == ERHIFilterMode::Nearest)
                        ? VK_SAMPLER_MIPMAP_MODE_NEAREST
                        : VK_SAMPLER_MIPMAP_MODE_LINEAR;

    // Addressing (wrap mode)
    info.addressModeU = VulkanRHI::ToVkAddressMode(Desc.AddressU);
    info.addressModeV = VulkanRHI::ToVkAddressMode(Desc.AddressV);
    info.addressModeW = VulkanRHI::ToVkAddressMode(Desc.AddressW);

    // LOD control
    info.mipLodBias = Desc.MipLodBias;
    info.minLod     = Desc.MinLod;
    info.maxLod     = Desc.MaxLod;

    // Anisotropy
    if (Desc.bEnableAnisotropy)
    {
        info.anisotropyEnable = VK_TRUE;
        info.maxAnisotropy    = Desc.MaxAnisotropy;
    }
    else
    {
        info.anisotropyEnable = VK_FALSE;
        info.maxAnisotropy    = 1.0f;
    }

    // Comparison (for shadow maps)
    if (Desc.bEnableComparison)
    {
        info.compareEnable = VK_TRUE;
        info.compareOp     = VulkanRHI::ToVkCompareOp(Desc.CompareOp);
    }
    else
    {
        info.compareEnable = VK_FALSE;
        info.compareOp     = VK_COMPARE_OP_ALWAYS;
    }

    // Border color
    info.borderColor = VulkanRHI::ToVkBorderColor(Desc.BorderColor);

    // Normalized coords (almost always true for textures)
    info.unnormalizedCoordinates = VK_FALSE;

    VkResult res = VulkanRHI::vkCreateSampler(Device.GetVkDevice(), &info, nullptr, &Sampler);
    RK_ASSERT(res == VK_SUCCESS, "Failed to create Vulkan sampler!");

    M_DescriptorHandle = Device.GetDescriptorManager()->AddSampler(Sampler);
}