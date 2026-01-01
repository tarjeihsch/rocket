#pragma once

#include <cstdint>

class RHISampler;

class RSampler
{
public:
    RSampler();
    ~RSampler();

    uint32_t GetBindingIndex() const;

private:
    RHISampler* Sampler;
};