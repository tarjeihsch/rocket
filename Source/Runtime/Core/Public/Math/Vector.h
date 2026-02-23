#pragma once

#include <algorithm>
#include <limits>

#include "MathTypes.h"

namespace Math
{
    template<typename T, size_t N>
    struct Vector
    {
        T M_Data[N] = {};

        Vector()
        {
        }

        Vector(T Value)
        {
            std::fill(M_Data, M_Data + N, Value);
        }

        void Zero()
        {
            memset(M_Data, 0, sizeof(T) * N);
        }
    };

    template<typename T>
    struct Vector<T, 3>
    {
        union
        {
            struct { T x; T y; T z; };
            struct { T r; T g; T b; };
            struct { T u; T v; T w; };

            T M_Data[3] = { 0, 0, 0 };
        };

        Vector()
        {
        }

        Vector(T P_Value)
        {
            M_Data[0] = P_Value;
            M_Data[1] = P_Value;
            M_Data[2] = P_Value;
        }

        Vector(T P_X, T P_Y, T P_Z)
        {
            M_Data[0] = P_X;
            M_Data[1] = P_Y;
            M_Data[2] = P_Z;
        }

        inline Vector<T, 3>& operator+=(const Vector<T, 3>& P_Other)
        {
            M_Data[0] += P_Other.M_Data[0];
            M_Data[1] += P_Other.M_Data[1];
            M_Data[2] += P_Other.M_Data[2];
            return *this;
        }

        inline Vector<T, 3>& operator-=(const Vector<T, 3>& P_Other)
        {
            M_Data[0] -= P_Other.M_Data[0];
            M_Data[1] -= P_Other.M_Data[1];
            M_Data[2] -= P_Other.M_Data[2];
            return *this;
        }

        inline Vector<T, 3>& operator*=(const Vector<T, 3>& P_Other)
        {
            M_Data[0] *= P_Other.M_Data[0];
            M_Data[1] *= P_Other.M_Data[1];
            M_Data[2] *= P_Other.M_Data[2];
            return *this;
        }

        inline Vector<T, 3>& operator/=(const Vector<T, 3>& P_Other)
        {
            M_Data[0] /= P_Other.M_Data[0];
            M_Data[1] /= P_Other.M_Data[1];
            M_Data[2] /= P_Other.M_Data[2];
            return *this;
        }

        inline Vector<T, 3>& operator+=(T P_Scalar)
        {
            M_Data[0] += P_Scalar;
            M_Data[1] += P_Scalar;
            M_Data[2] += P_Scalar;
            return *this;
        }

        inline Vector<T, 3>& operator-=(T P_Scalar)
        {
            M_Data[0] -= P_Scalar;
            M_Data[1] -= P_Scalar;
            M_Data[2] -= P_Scalar;
            return *this;
        }

        inline Vector<T, 3>& operator*=(T P_Scalar)
        {
            M_Data[0] *= P_Scalar;
            M_Data[1] *= P_Scalar;
            M_Data[2] *= P_Scalar;
            return *this;
        }

        inline Vector<T, 3>& operator/=(T P_Scalar)
        {
            T inv = T(1) / P_Scalar;
            M_Data[0] *= inv;
            M_Data[1] *= inv;
            M_Data[2] *= inv;
            return *this;
        }

        inline Vector<T, 3> operator+(const Vector<T, 3>& P_Other) const
        {
            return Vector<T, 3>(M_Data[0] + P_Other.M_Data[0], M_Data[1] + P_Other.M_Data[1], M_Data[2] + P_Other.M_Data[2]);
        }

        inline Vector<T, 3> operator-(const Vector<T, 3>& P_Other) const
        {
            return Vector<T, 3>(M_Data[0] - P_Other.M_Data[0], M_Data[1] - P_Other.M_Data[1], M_Data[2] - P_Other.M_Data[2]);
        }

        inline Vector<T, 3> operator*(const Vector<T, 3>& P_Other) const
        {
            return Vector<T, 3>(M_Data[0] * P_Other.M_Data[0], M_Data[1] * P_Other.M_Data[1], M_Data[2] * P_Other.M_Data[2]);
        }

        inline Vector<T, 3> operator/(const Vector<T, 3>& P_Other) const
        {
            return Vector<T, 3>(M_Data[0] / P_Other.M_Data[0], M_Data[1] / P_Other.M_Data[1], M_Data[2] / P_Other.M_Data[2]);
        }

        inline Vector<T, 3> operator+(T P_Scalar) const
        {
            return Vector<T, 3>(M_Data[0] + P_Scalar, M_Data[1] + P_Scalar, M_Data[2] + P_Scalar);
        }

        inline Vector<T, 3> operator-(T P_Scalar) const
        {
            return Vector<T, 3>(M_Data[0] - P_Scalar, M_Data[1] - P_Scalar, M_Data[2] - P_Scalar);
        }

        inline Vector<T, 3> operator*(T P_Scalar) const
        {
            return Vector<T, 3>(M_Data[0] * P_Scalar, M_Data[1] * P_Scalar, M_Data[2] * P_Scalar);
        }

        inline Vector<T, 3> operator/(T P_Scalar) const
        {
            return Vector<T, 3>(M_Data[0] / P_Scalar, M_Data[1] / P_Scalar, M_Data[2] / P_Scalar);
        }

        inline Vector<T, 3> operator-() const
        {
            return Vector<T, 3>(-M_Data[0], -M_Data[1], -M_Data[2]);
        }

        inline T Dot(const Vector<T, 3>& P_Other) const
        {
            return M_Data[0] * P_Other.M_Data[0] + M_Data[1] * P_Other.M_Data[1] + M_Data[2] * P_Other.M_Data[2];
        }

        inline Vector<T, 3> Cross(const Vector<T, 3>& P_Other) const
        {
            return Vector<T, 3>(
                M_Data[1] * P_Other.M_Data[2] - M_Data[2] * P_Other.M_Data[1],
                M_Data[2] * P_Other.M_Data[0] - M_Data[0] * P_Other.M_Data[2],
                M_Data[0] * P_Other.M_Data[1] - M_Data[1] * P_Other.M_Data[0]
            );
        }

        inline T Length() const
        {
            return std::sqrt(M_Data[0] * M_Data[0] + M_Data[1] * M_Data[1] + M_Data[2] * M_Data[2]);
        }

        inline T LengthSquared() const
        {
            return M_Data[0] * M_Data[0] + M_Data[1] * M_Data[1] + M_Data[2] * M_Data[2];
        }

        inline Vector<T, 3> Normalize() const
        {
            T Len = Length();

            if (Len < std::numeric_limits<T>::epsilon())
            {
                return Vector<T, 3>();
            }

            T Inv = T(1) / Len;
            return Vector<T, 3>(M_Data[0] * Inv, M_Data[1] * Inv, M_Data[2] * Inv);
        }

        void Zero()
        {
            M_Data[0] = T(0);
            M_Data[1] = T(0);
            M_Data[2] = T(0);
        }

        bool IsZero() const
        {
            return M_Data[0] == T(0) && M_Data[1] == T(0) && M_Data[2] == T(0);
        }

        static Vector<T, 3> Forward()   { return Vector<T, 3>(T(0), T(0), T(-1)); }
        static Vector<T, 3> Up()        { return Vector<T, 3>(T(0), T(1), T(0)); }
        static Vector<T, 3> Right()     { return Vector<T, 3>(T(1), T(0), T(0)); }
    };

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
}

using Vector3 = Math::Vector<Real, 3>;
using Vector3f = Math::Vector<float, 3>;
using Vector3d = Math::Vector<double, 3>;
using Vector3i = Math::Vector<int32_t, 3>;