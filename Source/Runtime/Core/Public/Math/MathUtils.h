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
        Result.Data[2][3] = T(1);
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
    inline T Lerp(const T a, const T b, T t)
    {
        return a + t * (b - a);
    }

    template<typename T>
    inline Vector<T, 3> Lerp(const Vector<T, 3>& a, const Vector<T, 3>& b, float t)
    {
        return a + t * (b - a);
    }

    template<typename T>
    inline Vector<T, 3> TransformPoint(const Vector<T, 3>& point, const Matrix<T, 4, 4>& matrix)
    {
        Vector<T, 4> vector = Vector<T, 4>(point.x, point.y, point.z, T(1.0));
        vector = matrix * vector;
        
        // Account for perspective division if the matrix contains projection data
        if (vector.w != T(1.0) && vector.w != T(0.0))
        {
            vector /= vector.w;
        }
        
        return Vector<T, 3>(vector.x, vector.y, vector.z); 
    }

    template<typename T>
    inline Vector<T, 3> TransformDirection(const Vector<T, 3>& velocity, const Matrix<T, 4, 4>& matrix)
    {
        Vector<T, 4> vector = Vector<T, 4>(velocity.x, velocity.y, velocity.z, T(0.0));
        vector = matrix * vector;
        return Vector<T, 3>(vector.x, vector.y, vector.z);
    }

    template<typename T>
    inline Vector<T, 3> LocalToWorldPoint(const Vector<T, 3>& local_point, const Matrix4x4& model_matrix)
    {
        return TransformPoint(local_point, model_matrix);
    }

    template<typename T>
    inline Vector<T, 3> WorldToLocalPoint(const Vector<T, 3>& world_point, const Matrix4x4& model_matrix)
    {
        return TransformPoint(world_point, model_matrix.Inverse());
    }

    inline Vector3 LocalToWorldDirection(const Vector3& local_dir, const Matrix4x4& model_matrix)
    {
        // Uses the direct model matrix, ignores translation (w = 0), and normalizes to strip object scale
        return TransformDirection(local_dir, model_matrix).Normalize();
    }

    inline Vector3 WorldToLocalDirection(const Vector3& world_dir, const Matrix4x4& model_matrix)
    {
        // Uses the inverse model matrix, ignores translation (w = 0), and normalizes to strip object scale
        return TransformDirection(world_dir, model_matrix.Inverse()).Normalize();
    }
}