#include <stddef.h>
#include "poly.h"
#include "math.h"
#include "camera.h"
#include <memory.h>

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
				Vector4D_Copy(&curr_poly->verts[j].v, &result);
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
				Vector4D_Copy(&curr_poly->tverts[vertex].v, &result);
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
			Vector4D_Copy(&object->vlist_local[vertex].v, &result);
		}
		break;
	case TRANSFORM_TRANS_ONLY:
		for (int vertex = 0; vertex < object->num_vertices; vertex++)
		{
			Vector4D result;
			Vector4D_PTR vector = &object->vlist_trans[vertex].v;
			Mat_Mul_Vector4D_4X4(vector, matrix, &result);
			Vector4D_Copy(&object->vlist_trans[vertex].v, &result);
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
		Vector4D_Copy(&object->ux, &result);

		//旋转uy
		Mat_Mul_Vector4D_4X4(&object->uy, matrix, &result);
		Vector4D_Copy(&object->uy, &result);

		//旋转uz
		Mat_Mul_Vector4D_4X4(&object->uz, matrix, &result);
		Vector4D_Copy(&object->uz, &result);
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

void Remove_Backfaces_RenderList4D(RenderList4D_PTR render_list, Camera4D_PTR camera)
{
	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		Poly4D_PTR curr_poly = render_list->poly_ptrs[poly];
		if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_CLIPPED || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->attr & POLY4D_ATTR_2SIDED)
			continue;

		Vector4D u, v, n;

		//计算法线向量
		Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
		Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

		Vector4D_Cross(&u, &v, &n);
		
		//创建指向视点的向量
		Vector4D view;
		Vector4D_Build(&curr_poly->tverts[0].v, &camera->pos, &view);

		float dp = Vector4D_Dot(&view, &n);

		if (dp <= 0.0)
		{
			SET_BIT(curr_poly->state, POLY4D_STATE_BACKFACE);
		}
	}
}

//flags表示要考虑的裁剪面
int Cull_Object4D(Object4D_PTR object, Camera4D_PTR camera, int cull_flags)
{
	Point4D sphere_pos;

	//1.将物体包围球的球心变为相机坐标
	Mat_Mul_Vector4D_4X4(&object->world_pos, &camera->mcam, &sphere_pos);

	if (cull_flags & CULL_OBJECT_Z_PLANE)
	{
		//使用远近裁剪面进行测试
		if (sphere_pos.z - object->max_radius > camera->far_clip_z || sphere_pos.z + object->max_radius < camera->near_clip_z)
			SET_BIT(object->state, OBJECT4D_STATE_CULLED);
		return 1;
	}

	if (cull_flags & CULL_OBJECT_Y_PLANE)
	{
		//这里用相似三角形来做，使用右裁剪面和左裁剪面检测包围球最上边和最右边的点(这里不完全正确，不过好在方法简单)
		float z_test = 0.5*camera->viewplane_height * sphere_pos.z / camera->view_dist_v;
		if (sphere_pos.y - object->max_radius > z_test || sphere_pos.y + object->max_radius < -z_test)
			SET_BIT(object->state, OBJECT4D_STATE_CULLED);
	}

	if (cull_flags & CULL_OBJECT_X_PLANE)
	{
		float z_test = 0.5*camera->viewplane_width * sphere_pos.z / camera->view_dist_h;
		if (sphere_pos.x - object->max_radius > z_test || sphere_pos.x + object->max_radius < -z_test)
			SET_BIT(object->state, OBJECT4D_STATE_CULLED);
	}
}

void Remove_Backfaces_Object4D(Object4D_PTR object, Camera4D_PTR camera)
{
	if (object->state & OBJECT4D_STATE_CULLED)
		return;

	for (int poly = 0; poly < object->num_polys; poly++)
	{
		PolyF4D_PTR curr_poly = &object->plist[poly];

		if (!(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_CLIPPED || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->attr & POLY4D_ATTR_2SIDED)
			continue;

		int vindex_0 = curr_poly->vert[0];
		int vindex_1 = curr_poly->vert[1];
		int vindex_2 = curr_poly->vert[2];

		//计算三角形的法线
		Vector4D u, v, n;
		Vector4D_Build(&object->vlist_trans[vindex_0].v, &object->vlist_trans[vindex_1].v, &u);
		Vector4D_Build(&object->vlist_trans[vindex_0].v, &object->vlist_trans[vindex_2].v, &v);
		Vector4D_Build(&u, &v, &n);

		//计算指向视点的向量
		Vector4D view;
		Vector4D_Build(&object->vlist_trans[0].v, &camera->pos, &view);

		float dp = Vector4D_Dot(&n, &view);
		if (dp <= 0.0)
			SET_BIT(object->state, POLY4D_STATE_BACKFACE);
	}
}

void Reset_RenderList4D(RenderList4D_PTR render_list)
{
	render_list->num_polys = 0;
}

//在渲染列表中插入多边形
int Insert_Poly4D_RenderList4D(RenderList4D_PTR render_list, Poly4D_PTR poly)
{
	if (render_list->num_polys >= RENDERLIST4D_MAX_POLYS)
		return 0;

	//将指针指向多边形结构
	render_list->poly_ptrs[render_list->num_polys] = &render_list->poly_data[render_list->num_polys];
	memcpy((void*)&render_list->poly_data[render_list->num_polys], (void *)poly, sizeof(Poly4D));

	if (render_list->num_polys == 0)
	{
		render_list->poly_data[0].next = NULL;
		render_list->poly_data[0].prev = NULL;
	}
	else
	{
		render_list->poly_data[render_list->num_polys].next = NULL;
		render_list->poly_data[render_list->num_polys-1].prev = &render_list->poly_data[render_list->num_polys];
	}

	render_list->num_polys++;

	return 1;
}