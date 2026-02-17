#pragma once

#include "MathTypes.h"

#include <stddef.h>
#include <cmath>
#include <limits>

template<typename T = Real>
struct Vector3
{
    union
    {
        struct { T x; T y; T z; };
        struct { T r; T g; T b; };
        struct { T u; T v; T w; };

        T data[3] { 0, 0, 0 };
    };

    Vector3()
    {
    }

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

    inline Vector3<T> Normalize() const
    {
        const T inverse_length = T(1) / Length();
        if (inverse_length < std::numeric_limits<T>::epsilon())
        {
            return Vector3<T>(0, 0, 0);
        }

        Vector3<T> result = Vector3<T>(data[0] * inverse_length, data[1] * inverse_length, data[2] * inverse_length);
        return result;
    }

    static Vector3<T> GetForwardVector();
    static Vector3<T> GetUpVector();
    static Vector3<T> GetRightVector();
};

using Vector3d = Vector3<double>;
using Vector3f = Vector3<float>;

template<typename T>
inline T DotProduct(const Vector3<T>& a, const Vector3<T>& b)
{
    T result = 0;
    result += a.data[0] * b.data[0];
    result += a.data[1] * b.data[1];
    result += a.data[2] * b.data[2];
    return result;
}

template<typename T>
inline Vector3<T> CrossProduct(const Vector3<T>& a, const Vector3<T>& b)
{
    Vector3<T> result;
    result.data[0] = (a.data[2] * b.data[3]) - (a.data[3] * b.data[2]);
    result.data[1] = (a.data[3] * b.data[1]) - (a.data[1] * b.data[3]);
    result.data[2] = (a.data[1] * b.data[2]) - (a.data[2] * b.data[1]);
    return result;
}

template<typename T>
inline T AngleBetween(Vector3<T> a, Vector3<T> b)
{
    T dot = DotProduct(a, b);
    T length = a.Length() * b.Length();
    T theta = dot / length;
    return std::acos(theta);
}

template<typename T>
inline Vector3<T> Vector3<T>::GetForwardVector()
{
    return Vector3<T>(0, 0, T(1));
}

template<typename T>
inline Vector3<T> Vector3<T>::GetUpVector()
{
    return Vector3<T>(0, T(1), 0);
}

template<typename T>
inline Vector3<T> Vector3<T>::GetRightVector()
{
    return Vector3<T>(T(1), 0, 0);
}