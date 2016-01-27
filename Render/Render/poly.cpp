#include <stddef.h>
#include "poly.h"
#include "math.h"

void Transform_RenderList4D(RenderList4D_PTR render_list, Matrix4X4_PTR matrix, int coord_selct)
{
	switch (coord_selct)
	{
	case TRANSFORM_LOCAL_ONLY:
		//只变换局部坐标，并将结果存储在局部坐标中
		for (int i = 0; i < render_list->num_polys; i++)
		{
			Poly4D_PTR curr_poly = render_list->poly_ptrs[i];
			if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->state & POLY4D_STATE_CLIPPED)
				continue;
			for (int j = 0; j < 3; j++)
			{
				Vector4D result;
				Mat_Mul_Vector4D_4X4(&curr_poly->verts[j].v, matrix, &result);
				Vector4D_COPY(&curr_poly->verts[j].v, &result);
			}
		}
		break;
	case TRANSFORM_LOCAL_TO_TRANS:
		//对局部顶点进行变换，然后把结果存储在变换列表中
		for (int i = 0; i < render_list->num_polys; i++)
		{
			Poly4D_PTR curr_poly = render_list->poly_ptrs[i];
			if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->state & POLY4D_STATE_CLIPPED)
				continue;
			for (int vertex = 0; vertex < 3; vertex++)
			{
				Mat_Mul_Vector4D_4X4(&curr_poly->verts[vertex].v, matrix, &curr_poly->tverts[vertex].v);
			}
		}
		break;
	case TRANSFORM_TRANS_ONLY:
		//对变换坐标进行变换
		for (int poly = 0; poly < render_list->num_polys; poly++)
		{
			Poly4D_PTR curr_poly = render_list->poly_ptrs[poly];
			if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->state & POLY4D_STATE_CLIPPED)
				continue;
			for (int vertex = 0; vertex < 3; vertex++)
			{
				Vector4D result;
				Mat_Mul_Vector4D_4X4(&curr_poly->tverts[vertex].v, matrix, &result);
				Vector4D_COPY(&curr_poly->tverts[vertex].v, &result);
			}
		}
		break;
	default:
		break;
	}
}

//transform_basis 表示是否要对朝向向量进行变换
void Transform_Object4D(Object4D_PTR object, Matrix4X4_PTR matrix, int coord_select,int transform_basis)
{
	switch (coord_select)
	{
	case TRANSFORM_LOCAL_ONLY:
		for (int vertex = 0; vertex < object->num_vertices; vertex++)
		{
			Vector4D result;
			Vector4D_PTR vector = &object->vlist_local[vertex].v;
			Mat_Mul_Vector4D_4X4(vector, matrix, &result);
			Vector4D_COPY(&object->vlist_local[vertex].v, &result);
		}
		break;
	case TRANSFORM_TRANS_ONLY:
		for (int vertex = 0; vertex < object->num_vertices; vertex++)
		{
			Vector4D result;
			Vector4D_PTR vector = &object->vlist_trans[vertex].v;
			Mat_Mul_Vector4D_4X4(vector, matrix, &result);
			Vector4D_COPY(&object->vlist_trans[vertex].v, &result);
		}
		break;
	case TRANSFORM_LOCAL_TO_TRANS:
		for (int vertex = 0; vertex < object->num_vertices; vertex++)
		{
			Mat_Mul_Vector4D_4X4(&object->vlist_local[vertex].v, matrix, &object->vlist_trans[vertex].v);
		}
		break;
	default:
		break;
	}

	if (transform_basis)
	{
		Vector4D result;

		//旋转ux
		Mat_Mul_Vector4D_4X4(&object->ux, matrix, &result);
		Vector4D_COPY(&object->ux, &result);

		//旋转uy
		Mat_Mul_Vector4D_4X4(&object->uy, matrix, &result);
		Vector4D_COPY(&object->uy, &result);

		//旋转uz
		Mat_Mul_Vector4D_4X4(&object->uz, matrix, &result);
		Vector4D_COPY(&object->uz, &result);
	}
}

void Build_Model_To_World_MATRIX4X4(Vector4D_PTR pos, Matrix4X4_PTR m)
{
	Mat_Init_4X4(m, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		pos->x, pos->y, pos->z, 1);
}

void Model_To_World_Object4D(Object4D_PTR object, Matrix4X4 matrix, int coord_select = TRANSFORM_LOCAL_TO_TRANS)
{

}