#pragma once

#include <array>
#include <stddef.h>
#include <cmath>

#include "Math.h"

template<typename T, size_t Count>
struct Vector
{
};

template<typename T>
struct Vector2 : Vector<T, 2>
{
    union
    {
        struct { T x; T y; };
        struct { T r; T g; };
        struct { T u; T v; };

        T data[2] { 0, 0 };
    };

    Vector2(T px, T py, T pz)
    {
    }

    // Magnitude of a vector is calculated using the Euclidaen norm (Pythagoras theorem generalized to N dimensions).
    inline T Length() const
    {
        return std::sqrt(std::pow(data[0], 2) + std::pow(data[1], 2));
    }
};

template<typename T>
struct Vector3 : Vector<T, 3>
{
    union
    {
        struct { T x; T y; T z; };
        struct { T r; T g; T b; };
        struct { T u; T v; T w; };

        T data[3] { 0, 0, 0 };
    };

    Vector3(T px, T py, T pz)
    {
        data[0] = px;
        data[1] = py;
        data[2] = pz;
    }

    inline Vector3<T>& operator+=(const Vector3<T>& other)
    {
        T x = data[0] + other.data[0];
        T y = data[1] + other.data[1];
        T z = data[2] + other.data[2];

        data[0] = x;
        data[1] = y;
        data[2] = z;

        return *this;
    }

    inline Vector3<T>& operator*=(T other)
    {
        T x = data[0] * other;
        T y = data[1] * other;
        T z = data[2] * other;

        data[0] = x;
        data[1] = y;
        data[2] = z;

        return *this;
    }

    inline Vector3<T>& operator*=(const Vector3<T>& other)
    {
        T x = data[0] * other.data[0];
        T y = data[1] * other.data[1];
        T z = data[2] * other.data[2];

        data[0] = x;
        data[1] = y;
        data[2] = z;

        return *this;
    }

    // Magnitude of a vector is calculated using the Euclidaen norm (Pythagoras theorem generalized to N dimensions).
    inline T Length() const
    {
        return std::sqrt(std::pow(data[0], 2) + std::pow(data[1], 2) + std::pow(data[2], 2));
    }
};

using Vector3d = Vector3<double>;
using Vector2d = Vector2<double>;

using Vector3f = Vector3<float>;
using Vector2f = Vector2<float>;

template<typename T>
static T DotProduct(Vector3<T> a, Vector3<T> b, T angle)
{
    T result = a.Length() * b.Length() * std::cos(angle);

    return result;
}

//template<typename T>
//static T CrossProduct(Vector3<T> a, Vector3<T> b, T angle, Vector3<T> unit)
//{
//    return result;
//}
