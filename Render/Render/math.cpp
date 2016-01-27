#include "math.h"
#include "poly.h"

void Mat_Mul_Vector4D_4X4(Vector4D_PTR va, Matrix4X4_PTR mb, Vector4D_PTR result)
{
	for (int col = 0; col < 4; col++)
	{
		float sum = 0;
		for (int row = 0; row < 4; row++)
		{
			sum += (va->M[row] * mb->M[row][col]);
		}
		result->M[col] = sum;
	}
}

void Mat_Init_4X4(Matrix4X4_PTR matrix,
	float m00, float m01, float m02, float m03,
	float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23,
	float m30, float m31, float m32, float m33)
{
	matrix->M00 = m00; matrix->M01 = m01; matrix->M02 = m02; matrix->M03 = m03;
	matrix->M10 = m10; matrix->M11 = m11; matrix->M12 = m12; matrix->M13 = m13;
	matrix->M20 = m00; matrix->M21 = m21; matrix->M22 = m22; matrix->M23 = m23;
	matrix->M30 = m30; matrix->M31 = m31; matrix->M32 = m32; matrix->M33 = m33;
}