#include "resource.h"
#include "poly.h"
#include "camera.h"
#include "math3d.h"
#include "light.h"
#include <stddef.h>
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

void Model_To_World_RenderList4D(RenderList4D_PTR rend_list,
	Point4D_PTR world_pos,
	int coord_select)
{
	// NOTE: Not matrix based
	// this function converts the local model coordinates of the
	// sent render list into world coordinates, the results are stored
	// in the transformed vertex list (tvlist) within the renderlist

	// interate thru vertex list and transform all the model/local 
	// coords to world coords by translating the vertex list by
	// the amount world_pos and storing the results in tvlist[]
	// is this polygon valid?

	if (coord_select == TRANSFORM_LOCAL_TO_TRANS)
	{
		for (int poly = 0; poly < rend_list->num_polys; poly++)
		{
			// acquire current polygon
			Poly4D_PTR curr_poly = rend_list->poly_ptrs[poly];

			// transform this polygon if and only if it's not clipped, not culled,
			// active, and visible, note however the concept of "backface" is 
			// irrelevant in a wire frame engine though
			if ((curr_poly == NULL) || !(curr_poly->state & POLY4D_STATE_ACTIVE) ||
				(curr_poly->state & POLY4D_STATE_CLIPPED) ||
				(curr_poly->state & POLY4D_STATE_BACKFACE))
				continue; // move onto next poly

			// all good, let's transform 
			for (int vertex = 0; vertex < 3; vertex++)
			{
				// translate vertex
				Vector4D_Add(&curr_poly->verts[vertex].v, world_pos, &curr_poly->tverts[vertex].v);

				//纹理坐标不会变换，直接复制
				Point2D_Copy(&curr_poly->tverts[vertex].t, &curr_poly->verts[vertex].t);
			} // end for vertex

		} // end for poly
	} // end if local
	else // TRANSFORM_TRANS_ONLY
	{
		for (int poly = 0; poly < rend_list->num_polys; poly++)
		{
			// acquire current polygon
			Poly4D_PTR curr_poly = rend_list->poly_ptrs[poly];

			// transform this polygon if and only if it's not clipped, not culled,
			// active, and visible, note however the concept of "backface" is 
			// irrelevant in a wire frame engine though
			if ((curr_poly == NULL) || !(curr_poly->state & POLY4D_STATE_ACTIVE) ||
				(curr_poly->state & POLY4D_STATE_BACKFACE) ||
				(curr_poly->state & POLY4D_STATE_CLIPPED))
				continue; // move onto next poly

			for (int vertex = 0; vertex < 3; vertex++)
			{
				// translate vertex
				Vector4D_Add(&curr_poly->tverts[vertex].v, world_pos, &curr_poly->tverts[vertex].v);

				//纹理坐标不会变换，直接复制
				Point2D_Copy(&curr_poly->tverts[vertex].t, &curr_poly->verts[vertex].t);
			} // end for vertex

		} // end for poly

	} // end else

} // end Model_To_World_RENDERLIST4DV1

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
		float z_test = 0.5f*camera->viewplane_height * sphere_pos.z / camera->view_dist_v;
		if (sphere_pos.y - object->max_radius > z_test || sphere_pos.y + object->max_radius < -z_test)
			SET_BIT(object->state, OBJECT4D_STATE_CULLED);
	}

	if (cull_flags & CULL_OBJECT_X_PLANE)
	{
		float z_test = 0.5f*camera->viewplane_width * sphere_pos.z / camera->view_dist_h;
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

void Clip_Poly_RenderList4D(RenderList4D_PTR renderlist, Camera4D_PTR cam, int clip_flags)
{
	//内部裁剪代码
#define CLIP_CODE_GZ	0x0001
#define CLIP_CODE_LZ	0x0002
#define CLIP_CODE_IZ	0x0004

#define CLIP_CODE_GX	0x0001
#define CLIP_CODE_LX	0x0002
#define CLIP_CODE_IX	0x0004

#define CLIP_CODE_GY	0x0001
#define CLIP_CODE_LY	0x0002
#define CLIP_CODE_IY	0x0004

#define CLIP_CODE_NULL	0x0000

	int vertex_codes[3];	//用于存储裁剪标记
	int num_verts_in;		//位于视景体内部的顶点数
	int v0, v1, v2;			//顶点索引

	float z_factor,			//用于裁剪计算
		z_test;				//用于裁剪计算

	float xi, yi, x1i, y1i, z1i, x2i, y2i,  //交点坐标
		t1, t2,								//参数化t值
		ui, vi, u1i, v1i, u2i, v2i;			//交点纹理坐标

	int last_poly_index,	//渲染列表中最后一个有效的多边形
		insert_poly_index;	//新多边形的插入位置

	Vector4D u, v, n;		//用于向量计算

	Poly4D tmp_poly;		//多边形分割成两个时，用于存储新增的多边形

	//设置last_poly_index 和 insert_poly_index ，使其对应于渲染列表末尾
	insert_poly_index = last_poly_index = renderlist->num_polys;

	for (int poly = 0; poly < last_poly_index; poly++)
	{
		Poly4D_PTR curr_poly = renderlist->poly_ptrs[poly];

		if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->state & POLY4D_STATE_CLIPPED)
			continue;

		if (clip_flags & CLIP_POLY_X_PLANE)
		{
			//只根据左、右裁剪面进行裁剪/剔除
			//对于每个顶点，判断它是否在裁剪区域内
			//并据此设置相应的裁剪码

			//由于是根据左右裁剪面来裁剪，因此需要根据FOV或裁剪面方程来确定
			z_factor = 0.5*cam->viewplane_width / cam->view_dist_h;

			//顶点0
			z_test = z_factor * curr_poly->tverts[0].z;

			if (curr_poly->tverts[0].x > z_test)
				vertex_codes[0] = CLIP_CODE_GX;
			else if (curr_poly->tverts[0].x < -z_test)
				vertex_codes[0] = CLIP_CODE_LX;
			else
				vertex_codes[0] = CLIP_CODE_IX;

			//顶点1
			z_test = z_factor * curr_poly->tverts[1].z;

			if (curr_poly->tverts[1].x > z_test)
				vertex_codes[1] = CLIP_CODE_GX;
			else if (curr_poly->tverts[0].x < -z_test)
				vertex_codes[1] = CLIP_CODE_LX;
			else
				vertex_codes[1] = CLIP_CODE_IX;

			//顶点2
			z_test = z_factor * curr_poly->tverts[2].z;

			if (curr_poly->tverts[2].x > z_test)
				vertex_codes[2] = CLIP_CODE_GX;
			else if (curr_poly->tverts[0].x < -z_test)
				vertex_codes[2] = CLIP_CODE_LX;
			else
				vertex_codes[2] = CLIP_CODE_IX;

			//进行简单的拒绝测试，即多边形是否完全在左/右裁剪面外侧
			if ((vertex_codes[0] == CLIP_CODE_GX && vertex_codes[1] == CLIP_CODE_GX && vertex_codes[2] == CLIP_CODE_GX) ||
				(vertex_codes[1] == CLIP_CODE_LX && vertex_codes[1] == CLIP_CODE_LX && vertex_codes[2] == CLIP_CODE_LX))

				SET_BIT(curr_poly->state, POLY4D_STATE_CLIPPED);
			continue;
		}

		if (clip_flags & CLIP_POLY_Y_PLANE)
		{
			z_factor = 0.5 * cam->viewplane_height / cam->view_dist_v;

			//顶点0
			z_test = z_factor * curr_poly->tverts[0].z;

			if (curr_poly->tverts[0].y > z_test)
				vertex_codes[0] = CLIP_CODE_GY;
			else if (curr_poly->tverts[0].y < -z_test)
				vertex_codes[0] = CLIP_CODE_LY;
			else
				vertex_codes[0] = CLIP_CODE_IY;

			//顶点1
			z_test = z_factor * curr_poly->tverts[1].z;

			if (curr_poly->tverts[1].y > z_test)
				vertex_codes[1] = CLIP_CODE_GY;
			else if (curr_poly->tverts[0].y < -z_test)
				vertex_codes[1] = CLIP_CODE_LY;
			else
				vertex_codes[1] = CLIP_CODE_IY;

			//顶点2
			z_test = z_factor * curr_poly->tverts[2].z;

			if (curr_poly->tverts[2].y > z_test)
				vertex_codes[2] = CLIP_CODE_GY;
			else if (curr_poly->tverts[0].y < -z_test)
				vertex_codes[2] = CLIP_CODE_LY;
			else
				vertex_codes[2] = CLIP_CODE_IY;

			//进行简单的拒绝测试，即多边形是否完全在上/下裁剪面外侧
			if ((vertex_codes[0] == CLIP_CODE_GY && vertex_codes[1] == CLIP_CODE_GY && vertex_codes[2] == CLIP_CODE_GY) ||
				(vertex_codes[1] == CLIP_CODE_LY && vertex_codes[1] == CLIP_CODE_LY && vertex_codes[2] == CLIP_CODE_LY))

				SET_BIT(curr_poly->state, POLY4D_STATE_CLIPPED);
			continue;
		}

		if (clip_flags & CLIP_POLY_Z_PLANE)
		{
			//重置内部顶点计数器，该变量用于对最终的三角形进行分类
			num_verts_in = 0;

			//顶点0
			if (curr_poly->tverts[0].z > cam->far_clip_z)
				vertex_codes[0] = CLIP_CODE_GZ;
			else if (curr_poly->tverts[0].z < cam->near_clip_z)
				vertex_codes[0] = CLIP_CODE_LZ;
			else
			{
				vertex_codes[0] = CLIP_CODE_IZ;
				num_verts_in++;
			}

			//顶点1
			if (curr_poly->tverts[1].z > cam->far_clip_z)
				vertex_codes[1] = CLIP_CODE_GZ;
			else if (curr_poly->tverts[1].z < cam->near_clip_z)
				vertex_codes[1] = CLIP_CODE_LZ;
			else
			{
				vertex_codes[1] = CLIP_CODE_IZ;
				num_verts_in++;
			}

			//顶点2
			if (curr_poly->tverts[2].z > cam->far_clip_z)
				vertex_codes[2] = CLIP_CODE_GZ;
			else if (curr_poly->tverts[2].z < cam->near_clip_z)
				vertex_codes[2] = CLIP_CODE_LZ;
			else
			{
				vertex_codes[2] = CLIP_CODE_IZ;
				num_verts_in++;
			}


			//进行简单的拒绝测试，即多边形是否完全在前/后裁剪面外侧
			if ((vertex_codes[0] == CLIP_CODE_GZ && vertex_codes[1] == CLIP_CODE_GZ && vertex_codes[2] == CLIP_CODE_GZ) ||
				(vertex_codes[1] == CLIP_CODE_LZ && vertex_codes[1] == CLIP_CODE_LZ && vertex_codes[2] == CLIP_CODE_LZ))

				SET_BIT(curr_poly->state, POLY4D_STATE_CLIPPED);
			continue;
		}

		//判断是否有在近裁剪面外侧
		if ((vertex_codes[0] | vertex_codes[1] | vertex_codes[2]) & CLIP_CODE_LZ)
		{
			//根据内侧顶点数对三角形分类

			//1，只有一个顶点在内部
			if (num_verts_in == 1)
			{

				//找出位于内侧的顶点
				if (vertex_codes[0] == CLIP_CODE_IZ)
				{
					v0 = 0; v1 = 1; v2 = 2;
				}
				else if (vertex_codes[1] == CLIP_CODE_IZ)
				{
					v0 = 1; v1 = 0; v2 = 1;
				}
				else
				{
					v0 = 2; v1 = 0; v2 = 1;
				}

				//对三角形边v0-v1进行裁剪
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->tverts[v1].v, &v);

				t1 = (cam->near_clip_z - curr_poly->tverts[v0].z) / v.z;
				
				//将t值带入x、y坐标分量方程，得到交点的x、y坐标
				xi = curr_poly->tverts[v0].x + v.x*t1;
				yi = curr_poly->tverts[v0].y + v.y*t1;

				//用交点覆盖原来的点
				curr_poly->tverts[v1].x = xi;
				curr_poly->tverts[v1].y = yi;
				curr_poly->tverts[v1].z = cam->near_clip_z;

				//对三角形v0-v2进行裁剪
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->tverts[v2].v, &v);

				t2 = (cam->near_clip_z - curr_poly->tverts[v0].z) / v.z;
				xi = curr_poly->tverts[v0].x + v.x * t1;
				yi = curr_poly->tverts[v0].y + v.y * t1;

				curr_poly->tverts[v2].x = xi;
				curr_poly->tverts[v2].y = yi;
				curr_poly->tverts[v2].z = cam->near_clip_z;

				//检查多边形是否带纹理
				if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_TEXTURE)
				{
					ui = curr_poly->tverts[v0].u0 + (curr_poly->tverts[v1].u0 - curr_poly->tverts[v0].u0)*t1;
					vi = curr_poly->tverts[v0].v0 + (curr_poly->tverts[v1].v0 - curr_poly->tverts[v0].v0)*t1;
					curr_poly->tverts[v1].u0 = ui;
					curr_poly->tverts[v1].v0 = vi;

					ui = curr_poly->tverts[v0].u0 + (curr_poly->tverts[v2].u0 - curr_poly->tverts[v0].u0)*t2;
					vi = curr_poly->tverts[v0].v0 + (curr_poly->tverts[v2].v0 - curr_poly->tverts[v0].v0)*t2;

					curr_poly->tverts[v2].u0 = ui;
					curr_poly->tverts[v2].v0 = vi;
				}

				//最后，重新计算法线长度
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->verts[v1].v, &u);
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->tverts[v2].v, &v);

				Vector4D_Cross(&u, &v, &n);
				curr_poly->nlength = Vector4D_Length(&n);
			}
			else if (num_verts_in == 2) //两个顶点在近裁剪面内，较为复杂
			{
				//裁剪之后为四边形，需要划分成两个三角形
				//因此我们用其中一个三角形覆盖原来的三角形，将另一个三角形插入到渲染列表末尾
				memcpy(&tmp_poly, curr_poly, sizeof(Poly4D));

				if(vertex_codes[0] == CLIP_CODE_LZ)
				{
					v0 = 0; v1 = 1; v2 = 2;
				}
				else if (vertex_codes[1] == CLIP_CODE_LZ)
				{
					v0 = 1; v1 = 0; v2 = 2;
				}
				else
				{
					v0 = 2; v1 = 0; v1 = 1;
				}

				//对v0->v1进行裁剪
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->tverts[v1].v, &v);
				t1 = (cam->near_clip_z - curr_poly->tverts[v0].z) / v.z;

				x1i = curr_poly->tverts[v0].x + v.x*t1;
				y1i = curr_poly->tverts[v0].x + v.y*t1;

				//对v0->v2进行裁剪
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->tverts[v1].v, &v);
				t2 = (cam->near_clip_z - curr_poly->tverts[v0].z) / v.z;

				x2i = curr_poly->tverts[v0].x + v.x * t2;
				y2i = curr_poly->tverts[v0].y + v.y * t2;

				curr_poly->tverts[v0].x = x1i;
				curr_poly->tverts[v0].y = y1i;
				curr_poly->tverts[v0].z = cam->near_clip_z;

				//现在需要使用两个交点和原来的顶点2创建一个新的三角形
				//该三角形将被插入刀渲染列表的末尾

				//因此v2保持不变
				tmp_poly.tverts[v1].x = x1i;
				tmp_poly.tverts[v1].y = y1i;
				tmp_poly.tverts[v1].z = cam->near_clip_z;

				tmp_poly.tverts[v0].x = x2i;
				tmp_poly.tverts[v0].y = y2i;
				tmp_poly.tverts[v0].z = cam->near_clip_z;

				//检查多边形是否带纹理，如果带纹理则对纹理进行裁剪
				if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_TEXTURE)
				{
					//计算多边形1的纹理坐标
					u1i = curr_poly->tverts[v0].u0 + (curr_poly->tverts[v1].u0 - curr_poly->tverts[v0].u0)*t1;
					v1i = curr_poly->tverts[v0].v0 + (curr_poly->tverts[v1].v0 - curr_poly->tverts[v0].v0)*t1;

					//计算多边形2的纹理坐标
					u2i = curr_poly->tverts[v0].u0 + (curr_poly->tverts[v1].u0 - curr_poly->tverts[v0].u0)*t2;
					v2i = curr_poly->tverts[v0].v0 + (curr_poly->tverts[v1].v0 - curr_poly->tverts[v0].v0)*t2;

					curr_poly->tverts[v0].u0 = u1i;
					curr_poly->tverts[v0].v0 = v1i;

					tmp_poly.tverts[v1].u0 = u1i;
					tmp_poly.tverts[v1].v0 = v1i;
					tmp_poly.tverts[v0].u0 = u2i;
					tmp_poly.tverts[v0].v0 = v2i;
				}

				//最后重新计算法线长度
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->tverts[v1].v, &u);
				Vector4D_Build(&curr_poly->tverts[v0].v, &curr_poly->tverts[v2].v, &v);

				Vector4D_Cross(&u, &v, &n);
				curr_poly->nlength = Vector4D_Length(&n);

				Vector4D_Build(&tmp_poly.tverts[v0].v, &tmp_poly.tverts[v1].v, &u);
				Vector4D_Build(&tmp_poly.tverts[v0].v, &tmp_poly.tverts[v1].v, &v);

				Vector4D_Cross(&u, &v, &n);

				tmp_poly.nlength = Vector4D_Length(&n);


				//将多边形插入到渲染列表末尾
				Insert_Poly4D_RenderList4D(renderlist, &tmp_poly);
			}
		}
	}
}

int Create_ZBuffer(ZBUFFER_PTR zb, int width, int height, int attr)
{
	if (!zb)
		return 0;

	if (zb->zbuffer)
		free(zb->zbuffer);

	zb->width = width;
	zb->height = height;
	zb->attr = attr;

	if (attr & ZBUFFER_ATTR_16BIT)
	{
		zb->sizeq = width*height / 2;
		if (zb->zbuffer = (UCHAR *)malloc(width * height * sizeof(USHORT)))
			return 1;
		else
			return 0;

	}
	else if (attr & ZBUFFER_ATTR_32BIT)
	{
		zb->sizeq = width * height;
		if (zb->zbuffer = (UCHAR *)malloc(width * height* sizeof(INT)))
			return 1;
		else
			return 0;
	}
	else
	{
		return 0;
	}
}

int Delete_ZBuffer(ZBUFFER_PTR zb)
{
	if (zb)
	{
		if (zb->zbuffer)
			free(zb->zbuffer);

		memset((void *)zb, 0, sizeof(ZBUFFER));

		return 1;

	}
	else
	{
		return 0;
	}
}

void Clear_ZBuffer(ZBUFFER_PTR zb, UINT data)
{
	Mem_Set_QUAD((UINT *)zb->zbuffer, data, zb->sizeq);
}