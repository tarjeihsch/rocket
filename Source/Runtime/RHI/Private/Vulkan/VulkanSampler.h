#pragma once
#include "Assertion.h"
#include "Common/RHISampler.h"
#include "VulkanDevice.h"
#include "VulkanLoader.h"
#include <vulkan/vulkan_core.h>

class VulkanSampler : public RHISampler
{
public:
    VulkanSampler(VulkanDevice& Device, RHISamplerDesc& Desc);
    ~VulkanSampler() override;

    virtual void* GetHandle() const override;
    virtual uint32_t GetBindingIndex() const override;

private:
    void Alloc();
    void Free();

private:
    RHISamplerDesc Desc;
    VkSampler Sampler;

    VulkanDevice& Device;
    BindlessHandle DescriptorHandle;
};
