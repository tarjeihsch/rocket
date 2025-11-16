#include "Sampler.h"

#include "Common/RHI.h"
#include "Common/RHISampler.h"

RSampler::RSampler()
{
    RRHISamplerDesc SamplerDesc = {};
    SamplerDesc.MinFilter = ERHIFilterMode::Linear;
    SamplerDesc.MagFilter = ERHIFilterMode::Linear;
    SamplerDesc.MipFilter = ERHIFilterMode::Linear;
    SamplerDesc.AddressU = ERHIAddressMode::ClampToEdge;
    SamplerDesc.AddressV = ERHIAddressMode::ClampToEdge;
    SamplerDesc.AddressW = ERHIAddressMode::ClampToEdge;
    SamplerDesc.MinLod = 0.0f;
    SamplerDesc.MaxLod = 0.0f;

    Sampler = GetRHI()->RHICreateSampler(SamplerDesc);
}

RSampler::~RSampler()
{
    delete Sampler;
}

uint32_t RSampler::GetBindingIndex() const
{
    return Sampler->GetBindingIndex();
}