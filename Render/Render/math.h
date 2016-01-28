#ifndef H_MATH
#define H_MATH

#include <math.h>

#define DEG_TO_RAD(degree) 3.1415926/360*degree
#define EPSILON_E3 (float)(1E-3) 
#define EPSILON_E4 (float)(1E-4) 
#define EPSILON_E5 (float)(1E-5)
#define EPSILON_E6 (float)(1E-6)

#define SET_BIT(word,bit_flag)   ((word)=((word) | (bit_flag)))

typedef struct Matrix4X4_TYP
{
	union
	{
		float M[4][4];

		struct 
		{
			float M00, M01, M02, M03;
			float M10, M11, M12, M13;
			float M20, M21, M22, M23;
			float M30, M31, M32, M33;
		};
	};
}Matrix4X4, *Matrix4X4_PTR;

void Mat_Mul_Vector4D_4X4(Vector4D_PTR va, Matrix4X4_PTR mb, Vector4D_PTR result);
void Mat_Mul_4X4(Matrix4X4_PTR ma, Matrix4X4_PTR mb, Matrix4X4_PTR result);

void Mat_Init_4X4(Matrix4X4_PTR matrix,
	float m00,float m01,float m02,float m03,
	float m10, float m11,float m12,float m13,
	float m20,float m21,float m22,float m23,
	float m30,float m31,float m32,float m33);

void Vector4D_InitXYZ(Vector4D_PTR vector, float x, float y, float z);
void Vector4D_Copy(Vector4D_PTR dest, Vector4D_PTR source);
void Vector4D_Zero(Vector4D_PTR vector);
void Vector4D_InitXYZ(Vector4D_PTR vector, float x, float y, float z);
void Vector4D_Build(Vector4D_PTR va, Vector4D_PTR vb, Vector4D_PTR result);
void Vector4D_Cross(Vector4D_PTR va, Vector4D_PTR vb, Vector4D_PTR result);
float Vector4D_Dot(Vector4D_PTR va, Vector4D_PTR vb);
void Vector4D_Normalize(Vector4D_PTR vector);

void Vector3D_Copy(Vector3D_PTR dest, Vector3D_PTR source);
void Vector3D_Zero(Vector3D_PTR vector);
void Vector3D_InitXYZ(Vector3D_PTR vector, float x, float y, float z);
float Vector3D_Length(Vector3D_PTR vector);
void Mat_Identity_4X4(Matrix4X4_PTR matrix);
void Vector3D_Normalize(Vector3D_PTR vector, Vector3D_PTR normal);
void Plane3D_Init(Plane3D_PTR plane, Point3D_PTR p0, Vector3D_PTR normal, int normalize = 0);

#endif