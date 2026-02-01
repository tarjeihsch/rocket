#pragma once

#include <cstdint>

class RHITextureSampler;

class Sampler
{
public:
    Sampler();
    ~Sampler();

    uint32_t GetBindingIndex() const;

private:
    RHITextureSampler* sampler;
};