#include "Sampler.h"

#include "Common/RHI.h"
#include "Common/RHISampler.h"

Sampler::Sampler()
{
    RHISamplerDesc SamplerDesc = {};
    SamplerDesc.MinFilter = ERHIFilterMode::Linear;
    SamplerDesc.MagFilter = ERHIFilterMode::Linear;
    SamplerDesc.MipFilter = ERHIFilterMode::Linear;
    SamplerDesc.AddressU = ERHIAddressMode::ClampToEdge;
    SamplerDesc.AddressV = ERHIAddressMode::ClampToEdge;
    SamplerDesc.AddressW = ERHIAddressMode::ClampToEdge;
    SamplerDesc.MinLod = 0.0f;
    SamplerDesc.MaxLod = 0.0f;

    sampler = GetRHI()->RHICreateSampler(SamplerDesc);
}

Sampler::~Sampler()
{
    delete sampler;
}

uint32_t Sampler::GetBindingIndex() const
{
    return sampler->GetBindingIndex();
}