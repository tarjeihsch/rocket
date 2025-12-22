#pragma once

#include "MathTypes.h"

#include <stddef.h>
#include <cmath>

template<typename T = Real>
struct Vector2
{
    union
    {
        struct { T x; T y; };
        struct { T r; T g; };
        struct { T u; T v; };

        T data[2] { 0, 0 };
    };

    Vector2()
    {
    }

    Vector2(T px, T py)
    {
        data[0] = px;
        data[1] = py;
    }

    // Magnitude of a vector is calculated using the Euclidaen norm (Pythagoras theorem generalized to N dimensions).
    inline T Length() const
    {
        return std::sqrt(std::pow(data[0], 2) + std::pow(data[1], 2));
    }

    // Normalize each component by dividing each component with respects to vector magnitude. Expect range between (-1, 1).
    inline Vector2<T> Normalize() const
    {
        T length = Length(); 
        Vector2<T> result;
        result.data[0] = data[0] / length;
        result.data[1] = data[1] / length;
        return result;
    }
};

using Vector2d = Vector2<double>;
using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;

template<typename T>
inline T DotProduct(const Vector2<T>& a, const Vector2<T>& b)
{
    T result = 0;
    result += a.data[0] * b.data[0];
    result += a.data[1] * b.data[1];
    return result;
}

template<typename T>
inline Vector2<T> CrossProduct(const Vector2<T>& a, const Vector2<T>& b)
{
    Vector2<T> result;
    result.data[0] = (a.data[2] * b.data[3]) - (a.data[3] * b.data[2]);
    result.data[1] = (a.data[3] * b.data[1]) - (a.data[1] * b.data[3]);
    return result;
}

template<typename T>
inline T AngleBetween(Vector2<T> a, Vector2<T> b)
{
    T dot = DotProduct(a, b);
    T length = a.Length() * b.Length();
    T theta = dot / length;
    return std::acos(theta);
}
