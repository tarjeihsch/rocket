#pragma once

#include "Math/MathTypes.h"
#include "Math/Vector.h"

namespace Math
{
	template <typename T, size_t R, size_t C>
	struct Matrix
	{
		T Data[R][C];

		Matrix()
		{
			memset(Data, 0, sizeof(T) * R * C);
		}

		static Matrix<T, R, C> Zero()
		{
			return Matrix<T, R, C>();
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

		Matrix<T, 4, 4> operator*(const Matrix<T, 4, 4>& Other)
		{
			Matrix<T, 4, 4> Result;

			for (uint32_t Row = 0; Row < 4; Row++)
			{
				for (uint32_t Column = 0; Column < 4; Column++)
				{
					Result.Data[Row][Column] = Data[Row][0]*Other.Data[0][Column]
						+ Data[Row][1]*Other.Data[1][Column]
						+ Data[Row][2]*Other.Data[2][Column]
						+ Data[Row][3]*Other.Data[3][Column];
				}
			}

			return Result;
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

			for (uint32_t Row = 0; Row < 4; ++Row)
			{
				for (uint32_t Column = 0; Column < 4; ++Column)
				{
					Result.Data[Column][Row] = Data[Row][Column];
				}
			}

			return Result;
		}
	};
}

using Matrix4x4 = Math::Matrix<Real, 4, 4>;
using Matrix4x4f = Math::Matrix<float, 4, 4>;
using Matrix4x4d = Math::Matrix<double, 4, 4>;