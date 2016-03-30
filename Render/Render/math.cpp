
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

void Mat_Mul_4X4(Matrix4X4_PTR ma, Matrix4X4_PTR mb, Matrix4X4_PTR result)
{
	for (int col = 0; col < 4; col++)
	{
		for (int row = 0; row < 4; row++)
		{
			float sum = 0;
			for (int index = 0; index < 4; index++)
			{
				sum += (ma->M[row][index] * mb->M[index][col]);
			}
			result->M[row][col] = sum;
		}
		
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

void Vector4D_Copy(Vector4D_PTR dest, Vector4D_PTR source)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
	dest->w = source->w;
}

void Vector4D_InitXYZ(Vector4D_PTR vector, float x, float y, float z)
{
	vector->x = x;
	vector->y = y;
	vector->z = z;
}

void Vector4D_Build(Vector4D_PTR va, Vector4D_PTR vb, Vector4D_PTR result)
{
	result->x = vb->x - va->x;
	result->y = vb->y - va->y;
	result->z = vb->z - va->z;
	result->w = 1;
}

void Vector4D_Cross(Vector4D_PTR va, Vector4D_PTR vb, Vector4D_PTR result)
{
	result->x = va->y * vb->z - va->z * vb->y;
	result->y = -(va->x*vb->z - va->z*vb->x);
	result->z = va->x*vb->y - va->y * vb->x;
	result->w = 1;
}

float Vector4D_Dot(Vector4D_PTR va, Vector4D_PTR vb)
{
	return va->x * vb->x + va->y * vb->y + va->z*vb->z;
}

void Vector4D_Normalize(Vector4D_PTR vector)
{
	float length = Vector3D_Length(vector);

	//如果是长度为0的向量
	if (length < EPSILON_E5)
	{
		return;
	}

	float length_inv = 1.0 / length;
	vector->x *= length_inv;
	vector->y *= length_inv;
	vector->z *= length_inv;
	vector->w = 1;
}

void Vector3D_InitXYZ(Vector3D_PTR vector, float x, float y, float z)
{
	vector->x = x;
	vector->y = y;
	vector->z = z;
}

void Vector4D_Zero(Vector4D_PTR vector)
{
	vector->x = 0;
	vector->y = 0;
	vector->z = 0;
}

void Vector4D_Div_By_W(Vector4D_PTR vector)
{
	vector->x /= vector->w;
	vector->y /= vector->y;
	vector->z /= vector->z;
	vector->w = 1;
}

void Point3D_Copy(Point3D_PTR dest, Point3D_PTR source)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
}

void Vector3D_Copy(Vector3D_PTR dest, Vector3D_PTR source)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
}

void Vector3D_Zero(Vector3D_PTR vector)
{
	vector->x = 0.0;
	vector->y = 0.0;
	vector->z = 0.0;
}

void Vector3D_Normalize(Vector3D_PTR vector, Vector3D_PTR normal)
{
	Vector3D_Zero(normal);
	float length = Vector3D_Length(vector);

	//如果是长度为0的向量
	if (length < EPSILON_E5)
	{
		return;
	}

	float length_inv = 1.0 / length;
	normal->x = vector->x * length_inv;
	normal->y = vector->y * length_inv;
	normal->z = vector->z * length_inv;
}

float Vector3D_Length(Vector3D_PTR vector)
{
	return sqrtf(vector->x * vector->x + vector->y*vector->y + vector->z*vector->z);
}

void Mat_Identity_4X4(Matrix4X4_PTR matrix)
{
	matrix->M00 = 1; matrix->M01 = 0; matrix->M02 = 0; matrix->M03 = 0;
	matrix->M00 = 0; matrix->M01 = 1; matrix->M02 = 0; matrix->M03 = 0;
	matrix->M00 = 0; matrix->M01 = 0; matrix->M02 = 1; matrix->M03 = 0;
	matrix->M00 = 0; matrix->M01 = 0; matrix->M02 = 0; matrix->M03 = 1;

}

void Plane3D_Init(Plane3D_PTR plane, Point3D_PTR p0, Vector3D_PTR normal, int normalize = 0)
{
	Point3D_Copy(&plane->p0, p0);
	if (!normalize)
	{
		Vector3D_Copy(&plane->n, normal);
	}
	else
	{
		Vector3D_Normalize(normal, &plane->n);
	}
}

//创建一个xyz的旋转矩阵
void Build_XYZ_Rotation_Matrix4X4(float theta_x, float theta_y, float theta_z, Matrix4X4_PTR matrix)
{
	Matrix4X4 mx, my, mz, mtmp;
	float sin_theta = 0, cos_theta = 0;
	int rot_seq = 0;	//1-x, 2-y, 4-z

	//重置为单位矩阵
	Mat_Identity_4X4(matrix);

	if (fabs(theta_x) > EPSILON_E5)
		rot_seq = rot_seq | 1;

	if (fabs(theta_y) > EPSILON_E5)
		rot_seq = rot_seq | 2;

	if (fabs(theta_z) > EPSILON_E5)
		rot_seq = rot_seq | 4;

	switch (rot_seq)
	{
	case 1: //x旋转
		cos_theta = cos(theta_x);
		sin_theta = sin(theta_x);

		Mat_Init_4X4(&mx, 1, 0, 0, 0,
			0, cos_theta, sin_theta, 0,
			0, -sin_theta, cos_theta, 0,
			0, 0, 0, 1);
		Mat_Copy_4X4(&mx, matrix);
		return;
		break;
	case 2:
		cos_theta = cos(theta_y);
		sin_theta = sin(theta_y);

		Mat_Init_4X4(&my, cos_theta, 0, -sin_theta, 0,
			0, 1, 0, 0,
			sin_theta, 0, cos_theta, 0,
			0, 0, 0, 1);

		Mat_Copy_4X4(&my, matrix);
		return;
		break;
	case 3: //xy旋转
		cos_theta = cos(theta_x);
		sin_theta = sin(theta_x);

		Mat_Init_4X4(&mx, 1, 0, 0, 0,
			0, cos_theta, sin_theta, 0,
			0, -sin_theta, cos_theta, 0,
			0, 0, 0, 1);

		cos_theta = cos(theta_y);
		sin_theta = sin(theta_y);

		Mat_Init_4X4(&my, cos_theta, 0, -sin_theta, 0,
			0, 1, 0, 0,
			sin_theta, 0, cos_theta, 0,
			0, 0, 0, 1);

		Mat_Mul_4X4(&mx, &my, matrix);
		break;
	case 4: //z旋转
		cos_theta = cos(theta_z);
		sin_theta = sin(theta_z);

		Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
			-sin_theta, cos_theta, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);

		Mat_Copy_4X4(&mz, matrix);
		return;
		break;
	case 5: //xz旋转
		cos_theta = cos(theta_x);
		sin_theta = sin(theta_x);

		Mat_Init_4X4(&mx, 1, 0, 0, 0,
			0, cos_theta, sin_theta, 0,
			0, -sin_theta, cos_theta, 0,
			0, 0, 0, 1);

		cos_theta = cos(theta_z);
		sin_theta = sin(theta_z);

		Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
			-sin_theta, cos_theta, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);

		Mat_Mul_4X4(&mx, &my, matrix);
		break;
	case 6: //yz旋转
		cos_theta = cos(theta_y);
		sin_theta = sin(theta_y);

		Mat_Init_4X4(&my, cos_theta, 0, -sin_theta, 0,
			0, 1, 0, 0,
			sin_theta, 0, cos_theta, 0,
			0, 0, 0, 1);

		cos_theta = cos(theta_z);
		sin_theta = sin(theta_z);

		Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
			-sin_theta, cos_theta, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		Mat_Mul_4X4(&my, &mz, matrix);
		break;
	case 7: //xyz旋转

		cos_theta = cos(theta_x);
		sin_theta = sin(theta_x);

		Mat_Init_4X4(&mx, 1, 0, 0, 0,
			0, cos_theta, sin_theta, 0,
			0, -sin_theta, cos_theta, 0,
			0, 0, 0, 1);

		cos_theta = cos(theta_y);
		sin_theta = sin(theta_y);

		Mat_Init_4X4(&my, cos_theta, 0, -sin_theta, 0,
			0, 1, 0, 0,
			sin_theta, 0, cos_theta, 0,
			0, 0, 0, 1);

		cos_theta = cos(theta_z);
		sin_theta = sin(theta_z);

		Mat_Init_4X4(&mz, cos_theta, sin_theta, 0, 0,
			-sin_theta, cos_theta, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);

		Mat_Mul_4X4(&mx, &my, &mtmp);
		Mat_Mul_4X4(&mtmp, &mz, matrix);
		break;
	default:
		break;
	}
}

