#pragma once
#include "Assertion.h"
#include "Common/RHITextureSampler.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include <vulkan/vulkan_core.h>

class VulkanTextureSampler : public RHITextureSampler
{
public:
    VulkanTextureSampler(VulkanDevice& Device, const RHITextureSamplerDesc& Desc);
    ~VulkanTextureSampler() override;

    virtual void* GetHandle() const override;
    virtual uint32_t GetBindingIndex() const override;

private:
    void Alloc();
    void Free();

private:
    RHITextureSamplerDesc Desc;
    VkSampler Sampler;

    VulkanDevice& Device;
    RHIDescriptorHandle DescriptorHandle;
};
