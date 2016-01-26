#include <stddef.h>
#include "poly.h"
#include "math.h"

void Transform_RenderList4D(RenderList4D_PTR render_list, Matrix4X4_PTR matrix, int coord_selct)
{
	switch (coord_selct)
	{
	case TRANSFORM_LOCAL_ONLY:
		//ֻ�任�ֲ����꣬��������洢�ھֲ�������
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
		//�Ծֲ�������б任��Ȼ��ѽ���洢�ڱ任�б���
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
		//�Ա任������б任
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