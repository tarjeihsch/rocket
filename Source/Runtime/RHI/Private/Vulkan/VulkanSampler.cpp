#include "VulkanSampler.h"

RVulkanSampler::RVulkanSampler(FVulkanDevice& InDevice, RRHISamplerDesc &InDesc)
    : Device(InDevice),
    Desc(InDesc),
    Sampler(VK_NULL_HANDLE)
{
    Alloc();
}
RVulkanSampler::~RVulkanSampler()
{
    Free();
}

void* RVulkanSampler::GetHandle() const
{
    return Sampler;
}

uint32_t RVulkanSampler::GetBindingIndex() const
{
    return DescriptorHandle.Index;
}

void RVulkanSampler::Alloc()
{
    VkSamplerCreateInfo SamplerCreateInfo = {};
    SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerCreateInfo.magFilter = VulkanRHI::ToVkFilter(Desc.MagFilter);
    SamplerCreateInfo.minFilter = VulkanRHI::ToVkFilter(Desc.MinFilter);
    SamplerCreateInfo.mipmapMode = (Desc.MipFilter == ERHIFilterMode::Nearest) ? VK_SAMPLER_MIPMAP_MODE_NEAREST : VK_SAMPLER_MIPMAP_MODE_LINEAR;
    SamplerCreateInfo.addressModeU = VulkanRHI::ToVkAddressMode(Desc.AddressU);
    SamplerCreateInfo.addressModeV = VulkanRHI::ToVkAddressMode(Desc.AddressV);
    SamplerCreateInfo.addressModeW = VulkanRHI::ToVkAddressMode(Desc.AddressW);
    SamplerCreateInfo.mipLodBias = Desc.MipLodBias;
    SamplerCreateInfo.minLod = Desc.MinLod;
    SamplerCreateInfo.maxLod = Desc.MaxLod;
    SamplerCreateInfo.borderColor = VulkanRHI::ToVkBorderColor(Desc.BorderColor);
    SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    if (Desc.bEnableAnisotropy)
    {
        SamplerCreateInfo.anisotropyEnable = VK_TRUE;
        SamplerCreateInfo.maxAnisotropy = Desc.MaxAnisotropy;
    }
    else
    {
        SamplerCreateInfo.anisotropyEnable = VK_FALSE;
        SamplerCreateInfo.maxAnisotropy = 1.0f;
    }

    if (Desc.bEnableComparison)
    {
        SamplerCreateInfo.compareEnable = VK_TRUE;
        SamplerCreateInfo.compareOp = VulkanRHI::ToVkCompareOp(Desc.CompareOp);
    }
    else
    {
        SamplerCreateInfo.compareEnable = VK_FALSE;
        SamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    }

    VkResult Result = VulkanRHI::vkCreateSampler(Device.GetVkDevice(), &SamplerCreateInfo, nullptr, &Sampler);
    RK_ASSERT(Result == VK_SUCCESS, "Failed to create sampler");

    DescriptorHandle = Device.GetDescriptorManager()->AddSampler(Sampler);
}

void RVulkanSampler::Free()
{
    Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::Sampler, Sampler, DescriptorHandle);
}