#pragma once

#include "Math/MathTypes.h"
#include "Math/Matrix.h"

#include "Math/Quaternion.h"
namespace Math
{
    // Right-handed perspective projection (Vulkan/GL clip space: depth [0,1], Y down for Vulkan)
    template<typename T>
    static Matrix<T, 4, 4> Perspective(T FOV_Y, T AspectRatio, T NearPlane, T FarPlane)
    {
        T H = T(1) / std::tan(FOV_Y * T(0.5));

        Matrix<T, 4, 4> Result;
        Result.Data[0][0] = H / AspectRatio;
        Result.Data[1][1] = -H;
        Result.Data[2][2] = FarPlane / (NearPlane - FarPlane);
        Result.Data[2][3] = T(-1);
        Result.Data[3][2] = (NearPlane * FarPlane) / (NearPlane - FarPlane);
        return Result;
    }

    // Infinite far plane reverse-Z perspective
    template<typename T>
    static Matrix<T, 4, 4> PerspectiveInfinite(T FOV_Y, T AspectRatio, T NearPlane)
    {
        T H = T(1) / std::tan(FOV_Y * T(0.5));

        Matrix<T, 4, 4> Result;
        Result.Data[0][0] = H / AspectRatio;
        Result.Data[1][1] = H;
        Result.Data[2][3] = T(-1);
        Result.Data[3][2] = NearPlane;
        return Result;
    }

    template<typename T>
    static Matrix<T, 4, 4> LookAt(const Vector<T, 3>& Position, T QX, T QY, T QZ, T QW)
    {
        Matrix<T, 4, 4> Rotation = Matrix<T, 4, 4>::FromQuaternion(QX, QY, QZ, QW);

        // View matrix is inverse of camera transform
        // For a rotation matrix, inverse = transpose
        Vector<T, 3> R(Rotation.Data[0][0], Rotation.Data[1][0], Rotation.Data[2][0]);
        Vector<T, 3> U(Rotation.Data[0][1], Rotation.Data[1][1], Rotation.Data[2][1]);
        Vector<T, 3> F(Rotation.Data[0][2], Rotation.Data[1][2], Rotation.Data[2][2]);

        Matrix<T, 4, 4> Result = Matrix<T, 4, 4>::Identity();
        Result.Data[0][0] = R.x; Result.Data[1][0] = R.y; Result.Data[2][0] = R.z;
        Result.Data[0][1] = U.x; Result.Data[1][1] = U.y; Result.Data[2][1] = U.z;
        Result.Data[0][2] = F.x; Result.Data[1][2] = F.y; Result.Data[2][2] = F.z;

        Result.Data[3][0] = -R.Dot(Position);
        Result.Data[3][1] = -U.Dot(Position);
        Result.Data[3][2] = -F.Dot(Position);

        return Result;
    }
    
    template<typename T>
    static Matrix<T, 4, 4> LookAt(const Vector<T, 3>& Position, const Vector<T, 3>& Rotation)
    {
        Quaternion<T> Q = Quaternion<T>::FromEuler(Rotation);
        Matrix<T, 4, 4> Rot = Q.ToMatrix();

        // 2. Extract basis vectors (Rows of the rotation matrix)
        Vector<T, 3> R(Rot.Data[0][0], Rot.Data[1][0], Rot.Data[2][0]);
        Vector<T, 3> U(Rot.Data[0][1], Rot.Data[1][1], Rot.Data[2][1]);
        Vector<T, 3> F(Rot.Data[0][2], Rot.Data[1][2], Rot.Data[2][2]);

        Matrix<T, 4, 4> Result = Matrix<T, 4, 4>::Identity();

        // Set the rotation part (Upper-left 3x3)
        Result.Data[0][0] = R.x; Result.Data[1][0] = R.y; Result.Data[2][0] = R.z;
        Result.Data[0][1] = U.x; Result.Data[1][1] = U.y; Result.Data[2][1] = U.z;
        Result.Data[0][2] = F.x; Result.Data[1][2] = F.y; Result.Data[2][2] = F.z;

        // Translation part: Project position onto the basis vectors
        Result.Data[3][0] = -R.Dot(Position);
        Result.Data[3][1] = -U.Dot(Position);
        Result.Data[3][2] = -F.Dot(Position);

        return Result;
    }
}