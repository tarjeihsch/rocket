#pragma once
#include "Assertion.h"
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

private:
    void Alloc();
    void Free();

private:
    RRHISamplerDesc Desc;
    VkSampler Sampler;

    FVulkanDevice& Device;
    RDescriptorHandle DescriptorHandle;
};
