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

void Mat_Init_4X4(Matrix4X4_PTR matrix,
	float m00,float m01,float m02,float m03,
	float m10, float m11,float m12,float m13,
	float m20,float m21,float m22,float m23,
	float m30,float m31,float m32,float m33);

inline void Vector4D_COPY(Vector4D_PTR dest, Vector4D_PTR source)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
	dest->w = source->w;
}

inline void Vector4D_InitXYZ(Vector4D_PTR vector, float x, float y, float z)
{
	vector->x = x;
	vector->y = y;
	vector->z = z;
}

inline void Vector4D_Zero(Vector4D_PTR vector)
{
	vector->x = 0;
	vector->y = 0;
	vector->z = 0;
}

inline void Mat_Identity_4X4(Matrix4X4_PTR matrix)
{
	matrix->M00 = 1; matrix->M01 = 0; matrix->M02 = 0; matrix->M03 = 0;
	matrix->M00 = 0; matrix->M01 = 1; matrix->M02 = 0; matrix->M03 = 0;
	matrix->M00 = 0; matrix->M01 = 0; matrix->M02 = 1; matrix->M03 = 0;
	matrix->M00 = 0; matrix->M01 = 0; matrix->M02 = 0; matrix->M03 = 1;

}