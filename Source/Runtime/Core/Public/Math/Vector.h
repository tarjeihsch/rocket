#pragma once

#include <algorithm>
#include <limits>
#include <string>

#include "MathTypes.h"

namespace Math
{
    template<typename T, size_t N>
    struct Vector
    {
        T data[N];

        Vector()
        {
            memset(&data, 0, sizeof(T) * N);
        }

        Vector(T Value)
        {
            std::fill(data, data + N, Value);
        }

        void Zero()
        {
            memset(data, 0, sizeof(T) * N);
        }
    };

    template<typename T>
    struct Vector<T, 2>
    {
        union
        {
            struct { T x; T y; };
            struct { T r; T g; };
            struct { T u; T v; };

            T data[2];
        };

        Vector()
        {
            memset(&data, 0, sizeof(T) * 2);
        }

        Vector(T pScalar)
        {
            data[0] = pScalar;
            data[1] = pScalar;
        }

        Vector(T pX, T pY)
        {
            data[0] = pX;
            data[1] = pY;
        }

        // Magnitude of a vector is calculated using the Euclidaen norm (Pythagoras theorem generalized to N dimensions).
        inline T Length() const
        {
            return std::sqrt(data[0] * data[0] + data[1] * data[1]);
        }

        inline T LengthSquared() const 
        {
            return data[0] * data[0] + data[1] * data[1];
        }

        // Normalize each component by dividing each component with respects to vector magnitude. Expect range between (-1, 1).
        inline Vector<T, 2> Normalize() const
        {
            Vector<T, 2> result;
            T length = Length();
            result.data[0] = data[0] / length;
            result.data[1] = data[1] / length;
            return result;
        }

        inline Vector<T, 2> DotProduct(const Vector<T, 2>& other)
        {
            return data[0] * other.data[0] + data[1] * other.data[1];
        }

        inline Vector<T, 2> CrossProduct(const Vector<T, 2>& other)
        {
            return data[0] * other.data[1] - data[1] * other.data[0];
        }

        static Vector<T, 2> Up() { return Vector<T, 2>(T(0), T(1)); }
        static Vector<T, 2> Right() { return Vector<T, 2>(T(1), T(0)); }
    };

    template<typename T>
    struct Vector<T, 3>
    {
        union
        {
            struct { T x; T y; T z; };
            struct { T r; T g; T b; };
            struct { T u; T v; T w; };

            T data[3];
        };

        Vector()
        {
            memset(&data, 0, sizeof(T) * 3);
        }

        Vector(T value)
        {
            data[0] = value;
            data[1] = value;
            data[2] = value;
        }

        Vector(T P_X, T P_Y, T P_Z)
        {
            data[0] = P_X;
            data[1] = P_Y;
            data[2] = P_Z;
        }

        inline Vector<T, 3>& operator+=(const Vector<T, 3>& other)
        {
            data[0] += other.data[0];
            data[1] += other.data[1];
            data[2] += other.data[2];
            return *this;
        }

        inline Vector<T, 3>& operator-=(const Vector<T, 3>& other)
        {
            data[0] -= other.data[0];
            data[1] -= other.data[1];
            data[2] -= other.data[2];
            return *this;
        }

        inline Vector<T, 3>& operator*=(const Vector<T, 3>& other)
        {
            data[0] *= other.data[0];
            data[1] *= other.data[1];
            data[2] *= other.data[2];
            return *this;
        }

        inline Vector<T, 3>& operator/=(const Vector<T, 3>& other)
        {
            data[0] /= other.data[0];
            data[1] /= other.data[1];
            data[2] /= other.data[2];
            return *this;
        }

        inline Vector<T, 3>& operator+=(T scalar)
        {
            data[0] += scalar;
            data[1] += scalar;
            data[2] += scalar;
            return *this;
        }

        inline Vector<T, 3>& operator-=(T scalar)
        {
            data[0] -= scalar;
            data[1] -= scalar;
            data[2] -= scalar;
            return *this;
        }

        inline Vector<T, 3>& operator*=(T scalar)
        {
            data[0] *= scalar;
            data[1] *= scalar;
            data[2] *= scalar;
            return *this;
        }

        inline Vector<T, 3>& operator/=(T scalar)
        {
            T inv = T(1) / scalar;
            data[0] *= inv;
            data[1] *= inv;
            data[2] *= inv;
            return *this;
        }

        inline Vector<T, 3> operator+(const Vector<T, 3>& other) const
        {
            return Vector<T, 3>(data[0] + other.data[0], data[1] + other.data[1], data[2] + other.data[2]);
        }

        inline Vector<T, 3> operator-(const Vector<T, 3>& other) const
        {
            return Vector<T, 3>(data[0] - other.data[0], data[1] - other.data[1], data[2] - other.data[2]);
        }

        inline Vector<T, 3> operator*(const Vector<T, 3>& other) const
        {
            return Vector<T, 3>(data[0] * other.data[0], data[1] * other.data[1], data[2] * other.data[2]);
        }

        inline Vector<T, 3> operator/(const Vector<T, 3>& other) const
        {
            return Vector<T, 3>(data[0] / other.data[0], data[1] / other.data[1], data[2] / other.data[2]);
        }

        inline Vector<T, 3> operator+(T scalar) const
        {
            return Vector<T, 3>(data[0] + scalar, data[1] + scalar, data[2] + scalar);
        }

        inline Vector<T, 3> operator-(T scalar) const
        {
            return Vector<T, 3>(data[0] - scalar, data[1] - scalar, data[2] - scalar);
        }

        inline Vector<T, 3> operator*(T scalar) const
        {
            return Vector<T, 3>(data[0] * scalar, data[1] * scalar, data[2] * scalar);
        }

        inline Vector<T, 3> operator/(T scalar) const
        {
            return Vector<T, 3>(data[0] / scalar, data[1] / scalar, data[2] / scalar);
        }

        inline Vector<T, 3> operator-() const
        {
            return Vector<T, 3>(-data[0], -data[1], -data[2]);
        }

        inline T Dot(const Vector<T, 3>& other) const
        {
            return data[0] * other.data[0] + data[1] * other.data[1] + data[2] * other.data[2];
        }

        inline Vector<T, 3> Cross(const Vector<T, 3>& other) const
        {
            return Vector<T, 3>(
                data[1] * other.data[2] - data[2] * other.data[1],
                data[2] * other.data[0] - data[0] * other.data[2],
                data[0] * other.data[1] - data[1] * other.data[0]
            );
        }

        inline T Length() const
        {
            return std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
        }

        inline T LengthSquared() const
        {
            return data[0] * data[0] + data[1] * data[1] + data[2] * data[2];
        }

        inline Vector<T, 3> Normalize() const
        {
            T length = Length();

            if (length < std::numeric_limits<T>::epsilon())
            {
                return Vector<T, 3>();
            }

            T reciprocal = T(1) / length;
            return Vector<T, 3>(data[0] * reciprocal, data[1] * reciprocal, data[2] * reciprocal);
        }

        void Zero()
        {
            data[0] = T(0);
            data[1] = T(0);
            data[2] = T(0);
        }

        bool IsZero() const
        {
            return data[0] == T(0) && data[1] == T(0) && data[2] == T(0);
        }

        static Vector<T, 3> Forward() { return Vector<T, 3>(T(0), T(0), T(-1)); }
        static Vector<T, 3> Up() { return Vector<T, 3>(T(0), T(1), T(0)); }
        static Vector<T, 3> Right() { return Vector<T, 3>(T(1), T(0), T(0)); }
        static Vector<T, 3> OneVector() { return Vector<T, 3>(T(1), T(1), T(1)); }
        static Vector<T, 3> ZeroVector() { return Vector<T, 3>(T(0), T(0), T(0)); }
    };

    template<typename T>
    struct Vector<T, 4>
    {
        union
        {
            struct { T x; T y; T z; T w; };
            struct { T r; T g; T b; T a; };

            T data[4];
        };

        Vector()
        {
            memset(&data, 0, sizeof(T) * 4);
        }

        Vector(T scalar)
        {
            data[0] = scalar;
            data[1] = scalar;
            data[2] = scalar;
            data[3] = scalar;
        }

        Vector(T scalarX, T scalarY, T scalarZ, T scalarW)
        {
            data[0] = scalarX;
            data[1] = scalarY;
            data[2] = scalarZ;
            data[3] = scalarW;
        }

        Vector(const Vector<T, 3>& vector, T scalarW)
        {
            data[0] = vector.data[0];
            data[1] = vector.data[1];
            data[2] = vector.data[2];
            data[3] = scalarW;
        }

        inline Vector<T, 4>& operator/=(T scalar)
        {
            T inv = T(1) / scalar;
            data[0] *= inv;
            data[1] *= inv;
            data[2] *= inv;
            data[3] *= inv;
            return *this;
        }

        inline Vector<T, 4> operator-(const Vector<T, 3>& other) const
        {
            return Vector<T, 4>(data[0] - other.data[0], data[1] - other.data[1], data[2] - other.data[2], data[3] - other.data[3]);
        }

        inline Vector<T, 4> operator-(T scalar) const
        {
            return Vector<T, 4>(data[0] - scalar, data[1] - scalar, data[2] - scalar);
        }

        static Vector<T, 4> Identity() 
        {
            return Vector<T, 4>(T(0), T(0), T(0), T(1));
        }
    };

    template<typename T>
    inline Vector<T, 3> operator*(T scalar, const Vector<T, 3>& vector)
    {
        return Vector<T, 3>(scalar * vector.x, scalar * vector.y, scalar * vector.z);
    }
    
    template<typename T>
    inline T DotProduct(const Vector<T, 3>& A, const Vector<T, 3>& B)
    {
        return A.Dot(B);
    }

    template<typename T>
    inline Vector<T, 3> CrossProduct(const Vector<T, 3>& A, const Vector<T, 3>& B)
    {
        return A.Cross(B);
    }

    template<typename T>
    inline T AngleBetween(const Vector<T, 3>& A, const Vector<T, 3>& B)
    {
        T Length_Product = std::sqrt(A.LengthSquared() * B.LengthSquared());
    
        if (Length_Product < std::numeric_limits<T>::epsilon())
        {
            return T(0);
        }
    
        return std::acos(Clamp(A.Dot(B) / Length_Product, T(-1), T(1)));
    }

    template<typename T>
    inline T DotProduct(const Vector<T, 2>& a, const Vector<T, 2>& b)
    {
        return a.DotProduct(b);
    }

    template<typename T>
    inline Vector<T, 2> CrossProduct(const Vector<T, 2>& a, const Vector<T, 2>& b)
    {
        return a.CrossProduct(b);
    }

    template<typename T>
    inline T AngleBetween(const Vector<T, 2>& a, const Vector<T, 2>& b)
    {
        T length = std::sqrt(a.LengthSquared() * b.LengthSquared());
        
        if (length < std::numeric_limits<T>::epsilon())
        {
            return T(0);
        }
        
        return std::acos(Clamp(a.DotProduct(b) / length, T(-1), T(1)));
    }
}

using Vector2 = Math::Vector<Real, 2>;
using Vector2f = Math::Vector<float, 2>;
using Vector2d = Math::Vector<double, 2>;
using Vector2i = Math::Vector<int32_t, 2>;

using Vector3 = Math::Vector<Real, 3>;
using Vector3f = Math::Vector<float, 3>;
using Vector3d = Math::Vector<double, 3>;
using Vector3i = Math::Vector<int32_t, 3>;

using Vector4 = Math::Vector<Real, 4>;
using Vector4f = Math::Vector<float, 4>;
using Vector4d = Math::Vector<double, 4>;
using Vector4i = Math::Vector<int32_t, 4>;
