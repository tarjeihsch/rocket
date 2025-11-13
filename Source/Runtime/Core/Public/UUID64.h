#pragma once

#include <cstdint>
#include <string>
#include <random>

namespace RUUID64Utils
{
    static std::random_device RandomDevice;
    static std::mt19937_64 Engine(RandomDevice());
    static std::uniform_int_distribution<uint64_t> UniformDistribution;
}

struct RUUID64
{
    RUUID64()
    {
        UUID = RUUID64Utils::UniformDistribution(RUUID64Utils::Engine);
    }

    RUUID64(uint32_t InUUID)
    {
        UUID = InUUID;
    }

    inline operator uint64_t() const
    {
        return UUID;
    }

private:
    uint64_t UUID;
};

namespace std
{
template <typename T> struct hash;

template<>
struct hash<RUUID64>
{
    std::size_t operator()(const RUUID64& UUID) const noexcept
    {
        return (uint64_t)UUID;
    }
};
};