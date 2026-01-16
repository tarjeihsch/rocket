#pragma once

#include <cstdint>

class RHISampler;

class Sampler
{
public:
    Sampler();
    ~Sampler();

    uint32_t GetBindingIndex() const;

private:
    RHISampler* sampler;
};