#pragma once

#include "Math/MathTypes.h"
#include "Math/Vector.h"

namespace Math
{
	template <typename T, size_t C, size_t R>
	struct Matrix
	{
		T Data[C][R];

		Matrix()
		{
			memset(Data, 0, sizeof(T) * C * R);
		}

		static Matrix<T, C, R> Zero()
		{
			return Matrix<T, C, R>();
		}
	};

	template <typename T>
	struct Matrix<T, 4, 4>
	{
		T Data[4][4];

		Matrix()
		{
			memset(Data, 0, sizeof(T) * 4 * 4);
		}

		static Matrix<T, 4, 4> Identity()
		{
			Matrix<T, 4, 4> Result;
			Result.Data[0][0] = T(1);
			Result.Data[1][1] = T(1);
			Result.Data[2][2] = T(1);
			Result.Data[3][3] = T(1);
			return Result;
		}

		Matrix<T, 4, 4> operator*(const Matrix<T, 4, 4>& Other) const
		{
			Matrix<T, 4, 4> Result;

			for (uint32_t Column = 0; Column < 4; ++Column)
			{
				for (uint32_t Row = 0; Row < 4; ++Row)
				{
					Result.Data[Column][Row] = Data[0][Row] * Other.Data[Column][0]
						+ Data[1][Row] * Other.Data[Column][1]
						+ Data[2][Row] * Other.Data[Column][2]
						+ Data[3][Row] * Other.Data[Column][3];
				}
			}

			return Result;
		}

		Vector<T, 4> operator*(const Vector<T, 4>& vector) const
		{
			Vector4 result;
			result.x = vector.x * Data[0][0] + vector.y * Data[1][0] + vector.z * Data[2][0] + vector.w * Data[3][0];
			result.y = vector.x * Data[0][1] + vector.y * Data[1][1] + vector.z * Data[2][1] + vector.w * Data[3][1];
			result.z = vector.x * Data[0][2] + vector.y * Data[1][2] + vector.z * Data[2][2] + vector.w * Data[3][2];
			result.w = vector.x * Data[0][3] + vector.y * Data[1][3] + vector.z * Data[2][3] + vector.w * Data[3][3];
			return result;
		}

		static Matrix<T, 4, 4> Translate(Vector<T, 3> translate)
		{
			Matrix<T, 4, 4> result = Matrix<T, 4, 4>::Identity();
			result.Data[3][0] = translate.x;
			result.Data[3][1] = translate.y;
			result.Data[3][2] = translate.z;
			return result;
		}

		static Matrix<T, 4, 4> Scale(Vector<T, 3> scale)
		{
			Matrix<T, 4, 4> result = Matrix<T, 4, 4>::Identity();
			result.Data[0][0] = scale.x;
			result.Data[1][1] = scale.y;
			result.Data[2][2] = scale.z;
			return result;
		}

		Matrix<T, 4, 4> Transpose() const
		{
			Matrix<T, 4, 4> Result;

			for (uint32_t Column = 0; Column < 4; ++Column)
			{
				for (uint32_t Row = 0; Row < 4; ++Row)
				{
					Result.Data[Column][Row] = Data[Row][Column];
				}
			}

			return Result;
		}

		Matrix<T, 4, 4> Inverse() const
		{
			Matrix<T, 4, 4> result;

			const T* m = &Data[0][0];

			T s0 = m[0] * m[5] - m[1] * m[4];
			T s1 = m[0] * m[6] - m[2] * m[4];
			T s2 = m[0] * m[7] - m[3] * m[4];
			T s3 = m[1] * m[6] - m[2] * m[5];
			T s4 = m[1] * m[7] - m[3] * m[5];
			T s5 = m[2] * m[7] - m[3] * m[6];

			T c0 = m[8] * m[13] - m[9] * m[12];
			T c1 = m[8] * m[14] - m[10] * m[12];
			T c2 = m[8] * m[15] - m[11] * m[12];
			T c3 = m[9] * m[14] - m[10] * m[13];
			T c4 = m[9] * m[15] - m[11] * m[13];
			T c5 = m[10] * m[15] - m[11] * m[14];

			T det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
			T inv = T(1) / det;

			T* r = &result.Data[0][0];

			r[0] = (m[5] * c5 - m[6] * c4 + m[7] * c3) * inv;
			r[1] = (-m[1] * c5 + m[2] * c4 - m[3] * c3) * inv;
			r[2] = (m[13] * s5 - m[14] * s4 + m[15] * s3) * inv;
			r[3] = (-m[9] * s5 + m[10] * s4 - m[11] * s3) * inv;

			r[4] = (-m[4] * c5 + m[6] * c2 - m[7] * c1) * inv;
			r[5] = (m[0] * c5 - m[2] * c2 + m[3] * c1) * inv;
			r[6] = (-m[12] * s5 + m[14] * s2 - m[15] * s1) * inv;
			r[7] = (m[8] * s5 - m[10] * s2 + m[11] * s1) * inv;

			r[8] = (m[4] * c4 - m[5] * c2 + m[7] * c0) * inv;
			r[9] = (-m[0] * c4 + m[1] * c2 - m[3] * c0) * inv;
			r[10] = (m[12] * s4 - m[13] * s2 + m[15] * s0) * inv;
			r[11] = (-m[8] * s4 + m[9] * s2 - m[11] * s0) * inv;

			r[12] = (-m[4] * c3 + m[5] * c1 - m[6] * c0) * inv;
			r[13] = (m[0] * c3 - m[1] * c1 + m[2] * c0) * inv;
			r[14] = (-m[12] * s3 + m[13] * s1 - m[14] * s0) * inv;
			r[15] = (m[8] * s3 - m[9] * s1 + m[10] * s0) * inv;

			return result;
		}
	};
}

using Matrix4x4 = Math::Matrix<Real, 4, 4>;
using Matrix4x4f = Math::Matrix<float, 4, 4>;
using Matrix4x4d = Math::Matrix<double, 4, 4>;