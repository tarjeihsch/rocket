#pragma once

#include "Math/Vector.h"
#include "Math/Matrix.h"

namespace Math
{
    template <typename T>
    struct Quaternion
    {
        union 
        {
            struct { T x, y, z, w; };
            T Data[4];
        };

        Quaternion() 
            : x(0)
            , y(0)
            , z(0)
            , w(1) 
        {
        }
    
        Quaternion(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) 
        {
        }

        static Quaternion<T> Identity()
        {
            return Quaternion<T>(0, 0, 0, 1);
        }

        // Standard Hamilton product (Order: Q1 * Q2)
        Quaternion<T> operator*(const Quaternion<T>& Other) const
        {
            return Quaternion<T>(
                w * Other.x + x * Other.w + y * Other.z - z * Other.y,
                w * Other.y - x * Other.z + y * Other.w + z * Other.x,
                w * Other.z + x * Other.y - y * Other.x + z * Other.w,
                w * Other.w - x * Other.x - y * Other.y - z * Other.z
            );
        }

        static Quaternion<T> FromEuler(const Vector<T, 3>& Euler)
        {
            T cx = std::cos(Euler.x * T(0.5));
            T sx = std::sin(Euler.x * T(0.5));
            T cy = std::cos(Euler.y * T(0.5));
            T sy = std::sin(Euler.y * T(0.5));
            T cz = std::cos(Euler.z * T(0.5));
            T sz = std::sin(Euler.z * T(0.5));

            Quaternion<T> q;
            // Order: Y (Yaw) -> X (Pitch) -> Z (Roll)
            q.x = sx * cy * cz - cx * sy * sz; // Flipped sign here
            q.y = cx * sy * cz + sx * cy * sz; // Flipped sign here
            q.z = cx * cy * sz - sx * sy * cz;
            q.w = cx * cy * cz + sx * sy * sz;
            return q;
        }

        Matrix<T, 4, 4> ToMatrix() const
        {
            Matrix<T, 4, 4> Result = Matrix<T, 4, 4>::Identity();
            T xx = x * x; T xy = x * y; T xz = x * z; T xw = x * w;
            T yy = y * y; T yz = y * z; T yw = y * w;
            T zz = z * z; T zw = z * w;

            // Column 0
            Result.Data[0][0] = T(1) - T(2) * (yy + zz);
            Result.Data[0][1] = T(2) * (xy + zw);
            Result.Data[0][2] = T(2) * (xz - yw);

            // Column 1
            Result.Data[1][0] = T(2) * (xy - zw);
            Result.Data[1][1] = T(1) - T(2) * (xx + zz);
            Result.Data[1][2] = T(2) * (yz + xw);

            // Column 2
            Result.Data[2][0] = T(2) * (xz + yw);
            Result.Data[2][1] = T(2) * (yz - xw);
            Result.Data[2][2] = T(1) - T(2) * (xx + yy);

            return Result;
        }
    };
}

using Quaternion = Math::Quaternion<Real>;