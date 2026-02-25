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
            struct
            {
                T w;
                T wi;
                T wj;
                T wk;
            };

            T Data[4];
        };

        Quaternion() 
            : w(1) 
            , wi(0)
            , wj(0)
            , wk(0)
        {
        }
    
        Quaternion(T pW, T pX, T pY, T pZ) 
            : w(pW) 
            , wi(pX)
            , wj(pY)
            , wk(pZ)
        {
        }

        T Magnitude() const
        {
            return std::sqrt(w*w + wi*wi + wj*wj + wk*wk);
        }

        Quaternion<T> Normalize() const
        {
            T magnitude = Magnitude();
            if (magnitude < T(1e-6))
            {
                return Quaternion<T>::Identity();
            }
            return Quaternion<T>(w / magnitude, wi / magnitude, wj / magnitude, wk / magnitude);
        }

        Quaternion<T> Conjugate() const
        {
            return Quaternion<T>(w, -wi, -wj, -wk);
        }

        Vector<T, 3> Rotate(const Vector<T, 3>& vector) const
        {
            Vector<T, 3> q(wi, wj, wk);
            return vector + (q.Cross(q.Cross(vector) + vector * w)) * T(2);
        }

        Quaternion<T> operator*(const Quaternion<T>& other) const
        {
            return Quaternion<T>(
                w * other.w - wi * other.wi - wj * other.wj - wk * other.wk,
                w * other.wi + wi * other.w + wj * other.wk - wk * other.wj,
                w * other.wj - wi * other.wk + wj * other.w + wk * other.wi,
                w * other.wk + wi * other.wj - wj * other.wi + wk * other.w
            );
        }

        Matrix<T, 4, 4> ToMatrix() const
        {
            Quaternion<T> quaternion = Normalize();
            Matrix<T, 4, 4> result = Matrix<T, 4, 4>::Identity();

            result.Data[0][0] = T(1) - T(2) * (quaternion.wj * quaternion.wj + quaternion.wk * quaternion.wk);
            result.Data[0][1] = T(2) * (quaternion.wi * quaternion.wj - quaternion.wk * quaternion.w);
            result.Data[0][2] = T(2) * (quaternion.wi * quaternion.wk + quaternion.wj * quaternion.w);

            result.Data[1][0] = T(2) * (quaternion.wi * quaternion.wj + quaternion.wk * quaternion.w);
            result.Data[1][1] = T(1) - T(2) * (quaternion.wi * quaternion.wi + quaternion.wk * quaternion.wk);
            result.Data[1][2] = T(2) * (quaternion.wj * quaternion.wk - quaternion.wi * quaternion.w);

            result.Data[2][0] = T(2) * (quaternion.wi * quaternion.wk - quaternion.wj * quaternion.w);
            result.Data[2][1] = T(2) * (quaternion.wj * quaternion.wk + quaternion.wi * quaternion.w);
            result.Data[2][2] = T(1) - T(2) * (quaternion.wi * quaternion.wi + quaternion.wj * quaternion.wj);

            return result;
        }

        static Quaternion<T> Identity()
        {
            return Quaternion<T>(1, 0, 0, 0);
        }

        static Quaternion<T> FromAxisAngle(const Vector<T, 3>& axis, T angle)
        {
            const T halfAngle = angle * T(0.5);
            const T s = std::sin(halfAngle);
            return Quaternion<T>(std::cos(halfAngle), axis.x * s, axis.y * s, axis.z * s).Normalize();
        }
    };
}

using Quaternion = Math::Quaternion<Real>;