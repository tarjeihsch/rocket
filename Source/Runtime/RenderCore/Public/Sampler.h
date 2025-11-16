#pragma once

#include <cstdint>

class RRHISampler;

class RSampler
{
public:
    RSampler();
    ~RSampler();

    uint32_t GetBindingIndex() const;

private:
    RRHISampler* Sampler;
};