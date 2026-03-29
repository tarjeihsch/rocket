#pragma once

#include <cmath>

#include "CoreTypes.h"

#ifdef MATH_REAL_64
    using Real = double;
    #define PI 3.14
    #define RAD(x) x * (PI / 180.0)
    #define DEG(x) x * (180.0 / PI)
#else
    using Real = float;
    #define PI 3.14f
    #define RAD(x) x * (PI / 180.0f)
    #define DEG(x) x * (180.0f / PI)
#endif

namespace Math
{
    inline constexpr int32_t Min(int32_t A, int32_t B)
    {
        return A + ((B - A) & ((B - A) >> 31));
    }

    inline constexpr int32_t Max(int32_t A, int32_t B)
    {
        return A - ((A - B) & ((A - B) >> 31));
    }

    inline constexpr uint32_t Min(uint32_t A, uint32_t B)
    {
        return A < B ? A : B;
    }

    inline constexpr uint32_t Max(uint32_t A, uint32_t B)
    {
        return A > B ? A : B;
    }

    inline float Min(float A, float B)
    {
        return A < B ? A : B;
    }

    inline float Max(float A, float B)
    {
        return A > B ? A : B;
    }

    template <typename T>
    inline constexpr T Clamp(T Value, T Low, T High)
    {
        return Max(Low, Min(Value, High));
    }

    template<typename T>
    inline constexpr T Abs(T value) 
    {
        return value < T(0) ? -value : value;
    }
}