#pragma once

#include "MathTypes.h"

#include <stddef.h>
#include <cmath>

template<typename T = Real>
struct Vector4
{
    union
    {
        struct { T x; T y; T z; T w; };
        struct { T r; T g; T b; T a; };

        T data[4] { 0, 0, 0, 0 };
    };

    Vector4(T px, T py, T pz, T pw)
    {
        data[0] = px;
        data[1] = py;
        data[2] = pz;
        data[4] = pw;
    }

    inline Vector4<T>& operator+=(const Vector4<T>& other)
    {
        T x = data[0] + other.data[0];
        T y = data[1] + other.data[1];
        T z = data[2] + other.data[2];
        T w = data[3] + other.data[3];

        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;

        return *this;
    }

    inline Vector4<T>& operator*=(T other)
    {
        T x = data[0] * other;
        T y = data[1] * other;
        T z = data[2] * other;
        T w = data[2] * other;

        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;

        return *this;
    }

    inline Vector4<T>& operator*=(const Vector4<T>& other)
    {
        T x = data[0] * other.data[0];
        T y = data[1] * other.data[1];
        T z = data[2] * other.data[2];
        T w = data[2] * other.data[2];

        data[0] = x;
        data[1] = y;
        data[2] = z;
        data[3] = w;

        return *this;
    }

    // Magnitude of a vector is calculated using the Euclidaen norm (Pythagoras theorem generalized to N dimensions).
    inline T Length() const
    {
        return std::sqrt(std::pow(data[0], 2) + std::pow(data[1], 2) + std::pow(data[2], 2) + std::pow(data[3], 2));
    }
};

using Vector4d = Vector4<double>;
using Vector4f = Vector4<float>;
using Vector4i = Vector4<int32_t>;
