#ifndef H_MATH3D
#define H_MATH3D

#include "poly.h"
//#include <Windows.h>

#define PI         ((float)3.141592654f)
#define DEG_TO_RAD(ang) ((ang)*PI/180.0)
#define EPSILON_E3 ((float)(1E-3)) 
#define EPSILON_E4 ((float)(1E-4))
#define EPSILON_E5 ((float)(1E-5))
#define EPSILON_E6 ((float)(1E-6))

#define SET_BIT(word,bit_flag) ((word) = ((word) | (bit_flag)))
#define Mat_Copy_4X4(src_mat, dest_mat) {memcpy((void *)(dest_mat), (void *)(src_mat), sizeof(Matrix4X4) ); }

void Mat_Mul_Vector4D_4X4(Vector4D_PTR va, Matrix4X4_PTR mb, Vector4D_PTR result);
void Mat_Mul_4X4(Matrix4X4_PTR ma, Matrix4X4_PTR mb, Matrix4X4_PTR result);

void Mat_Init_4X4(Matrix4X4_PTR matrix,
	float m00,float m01,float m02,float m03,
	float m10, float m11,float m12,float m13,
	float m20,float m21,float m22,float m23,
	float m30,float m31,float m32,float m33);

void Vector4D_InitXYZ(Vector4D_PTR vector, float x, float y, float z);
void Vector4D_Zero(Vector4D_PTR vector);
void Vector4D_Copy(Vector4D_PTR dest, Vector4D_PTR source);
void Vector4D_Build(Vector4D_PTR va, Vector4D_PTR vb, Vector4D_PTR result);
void Vector4D_Cross(Vector4D_PTR va, Vector4D_PTR vb, Vector4D_PTR result);
void Vector4D_Normalize(Vector4D_PTR vector);
void Vector4D_Div_By_W(Vector4D_PTR vector);
float Vector4D_Dot(Vector4D_PTR va, Vector4D_PTR vb);
float Vector4D_Length(Vector4D_PTR vector);
void Vector4D_Add(Vector4D_PTR va, Vector4D_PTR vb, Vector4D_PTR vsum);

void Point2D_Copy(Point2D_PTR dest, Point2D_PTR source);


void Vector3D_Copy(Vector3D_PTR dest, Vector3D_PTR source);
void Vector3D_Zero(Vector3D_PTR vector);
void Vector3D_InitXYZ(Vector3D_PTR vector, float x, float y, float z);
float Vector3D_Length(Vector3D_PTR vector);
void Mat_Identity_4X4(Matrix4X4_PTR matrix);
void Vector3D_Normalize(Vector3D_PTR vector, Vector3D_PTR normal);
void Plane3D_Init(Plane3D_PTR plane, Point3D_PTR p0, Vector3D_PTR normal, int normalize = 0);
void Build_XYZ_Rotation_Matrix4X4(float theta_x, float theta_y, float theta_z, Matrix4X4_PTR matrix);

inline void Mem_Set_WORD(unsigned int *dest, unsigned int data, int count)
{
	// this function fills or sets unsigned 16-bit aligned memory
	// count is number of words

	//Write_Error("{");

	for (int i = 0; i < count; i++)
	{
		dest ++;
		*dest = data;
	}

	//_asm
	//{
	//	mov edi, dest;			//edi points to destination memory
	//	mov ecx, count;			//number of 16 - bit words to move
	//	mov ax, data;			//16 - bit data
	//	rep stosw;				//move data
	//} // end asm

	//Write_Error("}");

} // end Mem_Set_WORD

inline void Mem_Set_QUAD(unsigned char* dest, unsigned int data, int count)
{
	// this function fills or sets unsigned 16-bit aligned memory
	// count is number of words

	//Write_Error("{");

    //qDebug()<<"mem reset";
	for (int i = 0; i < count; i++)
	{
		dest ++;
		*dest = data;
	}

	//_asm
	//{
	//	mov edi, dest;			//edi points to destination memory
	//	mov ecx, count;			//number of 16 - bit words to move
	//	mov ax, data;			//16 - bit data
	//	rep stosw;				//move data
	//} // end asm

	//Write_Error("}");

} // end Mem_Set_WORD


#endif
