#pragma once

#include "Math/MathTypes.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"

namespace Math
{
    template<typename T>
    inline Matrix<T, 4, 4> Perspective(T FOV_Y, T AspectRatio, T NearPlane, T FarPlane)
    {
        T FocalLength = T(1) / std::tan(FOV_Y * T(0.5));

        Matrix<T, 4, 4> Result;
        Result.Data[0][0] = FocalLength / AspectRatio;
        Result.Data[1][1] = FocalLength;
        Result.Data[2][2] = FarPlane / (NearPlane - FarPlane);
        Result.Data[2][3] = T(-1);
        Result.Data[3][2] = (NearPlane * FarPlane) / (NearPlane - FarPlane);
        return Result;
    }

    // Infinite reverse-Z maps near 1, far 0.
    template<typename T>
    inline Matrix<T, 4, 4> PerspectiveInfinity(T FOV_Y, T AspectRatio, T NearPlane)
    {
        T FocalLength = T(1) / std::tan(FOV_Y * T(0.5));

        Matrix<T, 4, 4> Result;
        Result.Data[0][0] = FocalLength / AspectRatio;
        Result.Data[1][1] = FocalLength;
        Result.Data[2][3] = T(-1);
        Result.Data[3][2] = NearPlane;
        return Result;
    }

    template<typename T>
    inline Matrix<T, 4, 4> Orthographic(T left, T right, T bottom, T top, T nearPlane, T farPlane)
    {
        Matrix<T, 4, 4> Result;
        
        // X-axis
        Result.Data[0][0] = T(2) / (right - left);
        
        // Y-axis
        Result.Data[1][1] = T(2) / (top - bottom);

        // Z-axis
        Result.Data[2][2] = T(1) / (nearPlane - farPlane);
        
        // Translation
        Result.Data[3][0] = -(right + left) / (right - left);
        Result.Data[3][1] = -(top + bottom) / (top - bottom);

        Result.Data[3][2] = -farPlane / (nearPlane - farPlane);
        Result.Data[3][3] = T(1);
        
        return Result;
    }

    template<typename T>
    inline Vector<T, 3> Lerp(const Vector<T, 3>& a, const Vector<T, 3>& b, float t)
    {
        return a + t * (b - a);
    }
}