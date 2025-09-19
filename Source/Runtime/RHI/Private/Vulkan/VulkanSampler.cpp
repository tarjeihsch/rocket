#include "VulkanSampler.h"
RVulkanSampler::RVulkanSampler(FVulkanDevice& InDevice, RRHISamplerDesc &InDesc)
    : Device(InDevice), Desc(InDesc)
{
    Alloc();
}
RVulkanSampler::~RVulkanSampler()
{
    Free();
}
void *RVulkanSampler::GetHandle() const
{
    return Sampler;
}

void RVulkanSampler::Free()
{
    Device.GetDeferredDeletionQueue()->EnqueueResource(FDeferredDeletionQueue::EType::Sampler, Sampler, M_DescriptorHandle);
}

uint32_t RVulkanSampler::GetBindingIndex() const
{
    return M_DescriptorHandle.Index;
}