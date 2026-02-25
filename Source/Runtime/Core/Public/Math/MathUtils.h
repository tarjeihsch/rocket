#pragma once

#include "Math/MathTypes.h"
#include "Math/Matrix.h"

#include "Math/Quaternion.h"
namespace Math
{
    template<typename T>
    static Matrix<T, 4, 4> Perspective(T FOV_Y, T AspectRatio, T NearPlane, T FarPlane)
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
    static Matrix<T, 4, 4> PerspectiveInfinity(T FOV_Y, T AspectRatio, T NearPlane)
    {
        T FocalLength = T(1) / std::tan(FOV_Y * T(0.5));

        Matrix<T, 4, 4> Result;
        Result.Data[0][0] = FocalLength / AspectRatio;
        Result.Data[1][1] = FocalLength;
        Result.Data[2][3] = T(-1);
        Result.Data[3][2] = NearPlane;
        return Result;
    }
}