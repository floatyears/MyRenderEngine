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

inline void Vector4D_COPY(Vector4D_PTR dest, Vector4D_PTR source)
{
	dest->x = source->x;
	dest->y = source->y;
	dest->z = source->z;
	dest->w = source->w;
}