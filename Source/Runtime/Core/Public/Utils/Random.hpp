#pragma once

#include <concepts>
#include <cstdint>

#include "Math/MathTypes.h"
#include "CoreTypes.h"
#include "Crypto.h"

namespace Random
{
    namespace Internal
    {
        // Modern inline thread_local with zero-overhead, thread-safe global state 
        // that safely persists per-thread without requiring a class wrapper.
        inline thread_local PCG32 GlobalEngine{ 0xDEADBEEFULL, 0x1ULL };
    }

    /**
     * @brief Reseeds the internal global pseudo-random engine.
     */
    inline void Seed(uint64_t Seed, uint64_t Sequence = 0)
    {
        Internal::GlobalEngine = PCG32(Seed, Sequence);
    }

    /**
     * @brief Generates a random integer within the closed interval [Min, Max].
     */
    template <std::integral T>
    inline T Range(T Min, T Max)
    {
        uint32_t RangeBound = static_cast<uint32_t>(Max - Min + 1);
        return Min + static_cast<T>(Internal::GlobalEngine.Next() % RangeBound);
    }

    /**
     * @brief Generates a random floating-point number within the half-open interval [Min, Max).
     */
    template <std::floating_point T>
    inline T Range(T Min, T Max)
    {
        // 4294967295U is UINT32_MAX
        T Normalized = static_cast<T>(Internal::GlobalEngine.Next()) / static_cast<T>(4294967295U);
        return Min + Normalized * (Max - Min);
    }

    /**
     * @brief Helper overload for the engine's default 'Real' type within the interval [Min, Max).
     */
    inline Real Range(Real Min, Real Max)
    {
        return Range<Real>(Min, Max);
    }

    /**
     * @brief Returns true or false based on a given probability (0.0 to 1.0).
     */
    inline bool NextBool(float Probability = 0.5f)
    {
        return Range<float>(0.0f, 1.0f) < Probability;
    }
}