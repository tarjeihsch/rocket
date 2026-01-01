#include "VulkanSampler.h"

VulkanSampler::VulkanSampler(VulkanDevice& InDevice, RHISamplerDesc &InDesc)
    : Device(InDevice),
    Desc(InDesc),
    Sampler(VK_NULL_HANDLE)
{
    Alloc();
}
VulkanSampler::~VulkanSampler()
{
    Free();
}

void* VulkanSampler::GetHandle() const
{
    return Sampler;
}

uint32_t VulkanSampler::GetBindingIndex() const
{
    return DescriptorHandle.Index;
}

void VulkanSampler::Alloc()
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

    VkResult Result = VulkanRHI::vkCreateSampler(Device.get_vk_device(), &SamplerCreateInfo, nullptr, &Sampler);
    ensure(Result == VK_SUCCESS, "Failed to create sampler");

    DescriptorHandle = Device.get_descriptor_manager()->AddSampler(Sampler);
}

void VulkanSampler::Free()
{
    Device.get_deferred_deletion_queue()->enqueue_resource(FDeferredDeletionQueue::EType::Sampler, Sampler, DescriptorHandle);
}