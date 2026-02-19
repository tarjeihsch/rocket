#pragma once

#include "MathTypes.h"
#include "Math/Vector3.h"

#include <assert.h>
#include <memory>

template<typename T = Real>
struct Matrix4x4
{
    T M_Data[4][4];

    Matrix4x4<T>()
    {
        memset(M_Data, 0, sizeof(M_Data));
    }

    static Matrix4x4<T> Identity()
    {
        Matrix4x4<T> result = {};
        result.M_Data[0][0] = T(1);
        result.M_Data[1][1] = T(1);
        result.M_Data[2][2] = T(1);
        result.M_Data[3][3] = T(1);
        return result;
    }

    T* operator[](uint32_t column) 
    {
        return M_Data[column];
    }
    
    const T* operator[](uint32_t column) const 
    { 
        return M_Data[column]; 
    }

    Matrix4x4 operator*(const Matrix4x4<T>& other) const
    {
        Matrix4x4 result;

        const T* column_0 = other.M_Data[0];
        const T* column_1 = other.M_Data[1];
        const T* column_2 = other.M_Data[2];
        const T* column_3 = other.M_Data[3];

        for (uint32_t row = 0; row < 4; ++row)
        {
            const T value_0 = M_Data[0][row];
            const T value_1 = M_Data[1][row];
            const T value_2 = M_Data[2][row];
            const T value_3 = M_Data[3][row];

            result[0][row] = value_0*column_0[0] + value_1*column_0[1] + value_2*column_0[2] + value_3*column_0[3];
            result[1][row] = value_0*column_1[0] + value_1*column_1[1] + value_2*column_1[2] + value_3*column_1[3];
            result[2][row] = value_0*column_2[0] + value_1*column_2[1] + value_2*column_2[2] + value_3*column_2[3];
            result[3][row] = value_0*column_3[0] + value_1*column_3[1] + value_2*column_3[2] + value_3*column_3[3];
        }

        return result;
    }

    Matrix4x4<T> Transpose() const
    {
        Matrix4x4<T> result;

        result.M_Data[0][0] = M_Data[0][0];
        result.M_Data[0][1] = M_Data[1][0];
        result.M_Data[0][2] = M_Data[2][0];
        result.M_Data[0][3] = M_Data[3][0];
        
        result.M_Data[1][0] = M_Data[0][1];
        result.M_Data[1][1] = M_Data[1][1];
        result.M_Data[1][2] = M_Data[2][1];
        result.M_Data[1][3] = M_Data[3][1];
        
        result.M_Data[2][0] = M_Data[0][2];
        result.M_Data[2][1] = M_Data[1][2];
        result.M_Data[2][2] = M_Data[2][2];
        result.M_Data[2][3] = M_Data[3][2];
        
        result.M_Data[3][0] = M_Data[0][3];
        result.M_Data[3][1] = M_Data[1][3];
        result.M_Data[3][2] = M_Data[2][3];
        result.M_Data[3][3] = M_Data[3][3];

        return result;
    }

    Matrix4x4<T> InverseAffine() const
    {
        assert(M_Data[0][3] == T(0) && M_Data[1][3] == T(0) && M_Data[2][3] == T(0) && M_Data[3][3] == T(1));

        Matrix4x4<T> Result;

        // Read upper-left 3x3
        const T C0_R0 = M_Data[0][0]; 
        const T C0_R1 = M_Data[0][1]; 
        const T C0_R2 = M_Data[0][2];

        const T C1_R0 = M_Data[1][0]; 
        const T C1_R1 = M_Data[1][1]; 
        const T C1_R2 = M_Data[1][2];

        const T C2_R0 = M_Data[2][0]; 
        const T C2_R1 = M_Data[2][1]; 
        const T C2_R2 = M_Data[2][2];
        
        const T C3_R0 = M_Data[3][0];
        const T C3_R1 = M_Data[3][1];
        const T C3_R2 = M_Data[3][2];

        const T Cofactor_C0_R0 = C1_R1 * C2_R2 - C1_R2 * C2_R1;
        const T Cofactor_C0_R1 = C1_R2 * C2_R0 - C1_R0 * C2_R2;
        const T Cofactor_C0_R2 = C1_R0 * C2_R1 - C1_R1 * C2_R0;

        const T Cofactor_C1_R0 = C0_R2 * C2_R1 - C0_R1 * C2_R2;
        const T Cofactor_C1_R1 = C0_R0 * C2_R2 - C0_R2 * C2_R0;
        const T Cofactor_C1_R2 = C0_R1 * C2_R0 - C0_R0 * C2_R1;

        const T Cofactor_C2_R0 = C0_R1 * C1_R2 - C0_R2 * C1_R1;
        const T Cofactor_C2_R1 = C0_R2 * C1_R0 - C0_R0 * C1_R2;
        const T Cofactor_C2_R2 = C0_R0 * C1_R1 - C0_R1 * C1_R0;

        const T Determinant = C0_R0 * Cofactor_C0_R0 + C0_R1 * Cofactor_C0_R1 + C0_R2 * Cofactor_C0_R2;
        assert(std::abs(Determinant) > T(1e-6));
        const T Inverse_Determinant = T(1) / Determinant;

        Result.M_Data[0][0] = Cofactor_C0_R0 * Inverse_Determinant;
        Result.M_Data[0][1] = Cofactor_C1_R0 * Inverse_Determinant;
        Result.M_Data[0][2] = Cofactor_C2_R0 * Inverse_Determinant;

        Result.M_Data[1][0] = Cofactor_C0_R1 * Inverse_Determinant;
        Result.M_Data[1][1] = Cofactor_C1_R1 * Inverse_Determinant;
        Result.M_Data[1][2] = Cofactor_C2_R1 * Inverse_Determinant;

        Result.M_Data[2][0] = Cofactor_C0_R2 * Inverse_Determinant;
        Result.M_Data[2][1] = Cofactor_C1_R2 * Inverse_Determinant;
        Result.M_Data[2][2] = Cofactor_C2_R2 * Inverse_Determinant;

        Result.M_Data[3][0] = -(Cofactor_C0_R0 * C3_R0 + Cofactor_C0_R1 * C3_R1 + Cofactor_C0_R2 * C3_R2) * Inverse_Determinant;
        Result.M_Data[3][1] = -(Cofactor_C1_R0 * C3_R0 + Cofactor_C1_R1 * C3_R1 + Cofactor_C1_R2 * C3_R2) * Inverse_Determinant;
        Result.M_Data[3][2] = -(Cofactor_C2_R0 * C3_R0 + Cofactor_C2_R1 * C3_R1 + Cofactor_C2_R2 * C3_R2) * Inverse_Determinant;

        return Result;
    }

    static Matrix4x4<T> Translate(const Vector3<T>& value)
    {
        Matrix4x4 result = Matrix4x4::Identity();
        result[3][0] = value.x;
        result[3][1] = value.y;
        result[3][2] = value.z;
        return result;
    }

    static Matrix4x4<T> Rotate(const Vector3<T>& value)
    {
        const T cos_x = cos(value.x);
        const T cos_y = cos(value.y);
        const T cos_z = cos(value.z);

        const T sin_x = sin(value.x);
        const T sin_y = sin(value.y);
        const T sin_z = sin(value.z);

        Matrix4x4<T> result_x = Matrix4x4<T>::Identity();
        result_x[1][1] = cos_x;
        result_x[1][2] = sin_x;
        result_x[2][1] = -sin_x;
        result_x[2][2] = cos_x;

        Matrix4x4<T> result_y = Matrix4x4<T>::Identity();
        result_y[0][0] = cos_y;
        result_y[0][2] = sin_y;
        result_y[2][0] = -sin_y;
        result_y[2][2] = cos_y;

        Matrix4x4<T> result_z = Matrix4x4<T>::Identity();
        result_z[0][0] = cos_z;
        result_z[0][1] = sin_z;
        result_z[1][0] = -sin_z;
        result_z[1][1] = cos_z;

        return result_z * result_x * result_y;
    }

    static Matrix4x4<T> Scale(const Vector3<T>& value)
    {
        Matrix4x4<T> result = Matrix4x4<T>::Identity();
        result[0][0] = value.x;
        result[1][1] = value.y;
        result[2][2] = value.z;
        return result;
    }
};

using Matrix4x4f = Matrix4x4<float>;
using Matrix4x4d = Matrix4x4<double>;