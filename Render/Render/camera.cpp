#include <stddef.h>
#include <math.h>
#include "math3d.h"
#include "camera.h"
#include "poly.h"

void Init_Camera4D(Camera4D_PTR camera,
	int cam_attr,
	Point4D_PTR cam_pos,
	Vector4D_PTR cam_dir,
	Point4D_PTR cam_target, //uvn����ĳ�ʼĿ��λ��
	float near_clip_z,
	float far_clip_z,
	float fov,
	float viewport_width,
	float viewport_height)
{
	camera->attr = cam_attr;
	Vector4D_Copy(&camera->pos, cam_pos);
	Vector4D_Copy(&camera->dir, cam_dir);

	//����uvn���
	Vector4D_InitXYZ(&camera->u, 1, 0, 0);
	Vector4D_InitXYZ(&camera->v, 0, 1, 0);
	Vector4D_InitXYZ(&camera->n, 0, 0, 1);

	if (cam_target != NULL) //uvnĿ��λ��
		Vector4D_Copy(&camera->target, cam_target);
	else
		Vector4D_Zero(&camera->target);
	
	camera->near_clip_z = near_clip_z;
	camera->far_clip_z = far_clip_z;

	camera->viewport_width = viewport_width;
	camera->viewport_height = viewport_height;
	camera->viewport_center_x = (viewport_width - 1) *0.5f;
	camera->viewport_center_y = (viewport_height - 1) *0.5f;

	camera->aspect_ratio = viewport_width / viewport_height;

	//�����б任��������Ϊ��λ����
	Mat_Identity_4X4(&camera->mcam);
	Mat_Identity_4X4(&camera->mper);
	Mat_Identity_4X4(&camera->mscr);

	camera->fov = fov;

	//��ƽ������Ϊ2x(2/ar)
	camera->viewplane_width = 2.0f;
	camera->viewplane_height = 2.0f / camera->aspect_ratio;

	float tan_fov_div2 = tan(DEG_TO_RAD(fov / 2));

	camera->view_dist_v = camera->view_dist_h = 0.5f * camera->viewplane_width * tan_fov_div2;
	 //= 0.5f * camera->viewplane_width * tan_fov_div2;

	if (fov == 90.0)
	{
		Point3D pt_origin; //�ü�����һ����
		Vector3D_InitXYZ(&pt_origin, 0, 0, 0);

		Vector3D vn; //�淨��

		//��ü���
		Vector3D_InitXYZ(&vn, 1, 0, -1); //ƽ��x=z
		Plane3D_Init(&camera->rt_clip_plane, &pt_origin, &vn, 1);

		//�Ҳü���
		Vector3D_InitXYZ(&vn, -1, 0, -1);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);

		//�ϲü���
		Vector3D_InitXYZ(&vn, 0, 1, -1);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);

		//�²ü���
		Vector3D_InitXYZ(&vn, 0, -1, -1);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);
	}
	else
	{
		//����fov��Ϊ90�ȵ����
		Point3D pt_origin; //�ü�����һ����
		Vector3D_InitXYZ(&pt_origin, 0, 0, 0);

		Vector3D vn;

		Vector3D_InitXYZ(&vn, camera->view_dist_h, 0, -camera->viewplane_width / 2.0f);
		Plane3D_Init(&camera->rt_clip_plane, &pt_origin, &vn, 1);

		Vector3D_InitXYZ(&vn, -camera->view_dist_h, 0, -camera->viewplane_width / 2.0f);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);

		Vector3D_InitXYZ(&vn, 0, camera->view_dist_h, -camera->viewplane_width / 2.0f);
		Plane3D_Init(&camera->tp_clip_plane, &pt_origin, &vn, 1);

		Vector3D_InitXYZ(&vn, 0, -camera->view_dist_h, -camera->viewplane_width / 2.0f);
		Plane3D_Init(&camera->bt_clip_plane, &pt_origin, &vn, 1);
	}
}

void Build_Camera4D_Matrix_Euler(Camera4D_PTR camera, int camera_rotation_seq)
{
	Matrix4X4 mt_inv,		//���ƽ�ƾ���������
		mx_inv,				//�����x�����ת����������
		my_inv,				//�����y�����ת����������
		mz_inv,				//�����z�����ת����������
		mrot,				//��������ת����Ļ�
		mtmp;				//���ڴ洢��ʱ����

	//�����������任����
	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-camera->pos.x, -camera->pos.y, -camera->pos.z, 1);

	//1.��ȡŷ���Ƕ�
	float theta_x = camera->dir.x;
	float theta_y = camera->dir.y;
	float theta_z = camera->dir.z;

	float cos_theta = cos(theta_x);
	float sin_theta = -sin(theta_x);

	//��������
	Mat_Init_4X4(&mx_inv, 1, 0, 0, 0,
		0, cos_theta, sin_theta, 0,
		0, -sin_theta, cos_theta, 0,
		0, 0, 0, 1);

	cos_theta = cos(theta_y);
	sin_theta = -sin(theta_y);

	//��������
	Mat_Init_4X4(&my_inv, cos_theta,0, -sin_theta, 0,
		0, 1, 0, 0,
		sin_theta, 0, cos_theta, 0,
		0, 0, 0, 1);

	cos_theta = cos(theta_z);
	sin_theta = -sin(theta_z);

	//��������
	Mat_Init_4X4(&mz_inv, cos_theta, sin_theta, 0, 0,
		-sin_theta, cos_theta, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	switch (camera_rotation_seq)
	{
	case CAM_ROT_SEQ_XYZ:
		Mat_Mul_4X4(&mx_inv, &my_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mz_inv, &mrot);
		break;
	case CAM_ROT_SEQ_XZY:
		Mat_Mul_4X4(&mx_inv, &mz_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &my_inv, &mrot);
		break;
	case CAM_ROT_SEQ_YXZ:
		Mat_Mul_4X4(&my_inv, &mx_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mz_inv, &mrot);
		break;
	case CAM_ROT_SEQ_YZX:
		Mat_Mul_4X4(&my_inv, &mz_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mx_inv, &mrot);
		break;
	case CAM_ROT_SEQ_ZXY:
		Mat_Mul_4X4(&mz_inv, &mx_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &my_inv, &mrot);
		break;
	case CAM_ROT_SEQ_ZYX:
		Mat_Mul_4X4(&mz_inv, &my_inv, &mtmp);
		Mat_Mul_4X4(&mtmp, &mx_inv, &mrot);
		break;
	default:
		break;
	}

	Mat_Mul_4X4(&mt_inv, &mrot, &camera->mcam);
}

void Build_Camera4D_Matrix_UVN(Camera4D_PTR camera, int mode)
{
	Matrix4X4 mt_inv,		//�����ƽ�ƾ���
		mt_uvn,				//UVN����任����
		mtmp;				//���ڴ洢��ʱ����

	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-camera->pos.x, -camera->pos.y, -camera->pos.z, 1);

	//ȷ����μ���Ŀ���
	if (mode == UVN_MODE_SPHERICAL)
	{
		float phi = camera->dir.x;		//����
		float theta = camera->dir.y;	//��λ��

		float sin_phi = sin(phi);
		float cos_phi = cos(phi);

		float sin_theta = sin(theta);
		float cos_theta = cos(theta);

		//����Ŀ����ڵ�λ�����ϵ�λ�ã�x,y��z��
		camera->target.x = -1 * sin_phi * sin_theta;
		camera->target.y = 1 * cos_phi;
		camera->target.z = 1 * sin_phi * cos_theta;
	}

	//����������˿������¼���UVN�����ȫ���������۲�ο����Ŀ���
	//1. n = <Ŀ��λ��-�۲�ο���>
	Vector4D_Build(&camera->pos, &camera->target, &camera->n);

	//2. ��v����Ϊ<0,1,0>
	Vector4D_InitXYZ(&camera->v, 0, 1, 0);

	//3. u = (v x n)
	Vector4D_Cross(&camera->v, &camera->n, &camera->u);

	//4. v = (n x u)
	Vector4D_Cross(&camera->n, &camera->u, &camera->v);

	//5. ��һ����������
	Vector4D_Normalize(&camera->u);
	Vector4D_Normalize(&camera->v);
	Vector4D_Normalize(&camera->n);

	//��uvn���룬�õ�UVN��ת����
	Mat_Init_4X4(&mt_uvn, camera->u.x, camera->v.x, camera->n.x, 0,
		camera->u.y, camera->v.y, camera->n.y, 0,
		camera->u.z, camera->v.z, camera->n.z, 0,
		0, 0, 0, 1);

	//��ƽ�ƾ������uvn����
	Mat_Mul_4X4(&mt_inv, &mt_uvn, &camera->mcam);
}

void Camera_To_Perspective_Object4D(Object4D_PTR object, Camera4D_PTR camera)
{
	for (int vertex = 0; vertex < object->num_vertices; vertex++)
	{
		float z = object->vlist_trans[vertex].z;

		object->vlist_trans[vertex].x = camera->view_dist_h * object->vlist_trans[vertex].x / z;
		object->vlist_trans[vertex].y = camera->view_dist_v * object->vlist_trans[vertex].y * camera->aspect_ratio / z;
	}
}

void Convert_From_Homogeneous4D_Object4D(Object4D_PTR object)
{
	for (int vertex = 0; vertex < object->num_vertices; vertex++)
	{
		Vector4D_Div_By_W(&object->vlist_trans[vertex].v);
	}
}

void Camera_To_Perspective_RenderList4D(RenderList4D_PTR render_list, Camera4D_PTR camera)
{
	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		Poly4D_PTR curr_poly = render_list->poly_ptrs[poly];
		if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_CLIPPED || curr_poly->state & POLY4D_STATE_BACKFACE)
			continue;
		for (int vertex = 0; vertex < 3; vertex++)
		{
			float z = curr_poly->tverts[vertex].z;
			curr_poly->tverts[vertex].x = camera->view_dist_h * curr_poly->tverts[vertex].x / z;
			curr_poly->tverts[vertex].y = camera->view_dist_v * curr_poly->tverts[vertex].y * camera->aspect_ratio / z;

			//Ϊ���ں���ʹ��1/z���棬����Ԥ�ȴ�������
			curr_poly->tverts[vertex].z = 1 / curr_poly->tverts[vertex].z;
		}
	}
}

void Convert_From_Homogeneous4D_RenderList4D(RenderList4D_PTR render_list)
{
	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		Poly4D_PTR curr_poly = render_list->poly_ptrs[poly];
		if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_CLIPPED || curr_poly->state & POLY4D_STATE_BACKFACE)
			continue;
		for (int vertex = 0; vertex < 3; vertex++)
		{
			Vector4D_Div_By_W(&curr_poly->tverts[vertex].v);
		}
	}
}


void Build_Camera_To_Perspective_Matrix4X4(Camera4D_PTR camera, Matrix4X4_PTR matrix)
{
	Mat_Init_4X4(matrix, camera->view_dist_h, 0, 0, 0,
		0, camera->view_dist_v*camera->aspect_ratio, 0, 0,
		0, 0, 1, 1,
		0, 0, 0, 0);
}

void Perspective_To_Screen_Object4D(Object4D_PTR object, Camera4D_PTR camera)
{
	float alpha = 0.5f*camera->viewplane_width - 0.5f;
	float beta = 0.5f*camera->viewplane_height - 0.5f;

	for (int vertex = 0; vertex < object->num_vertices; vertex++)
	{
		object->vlist_trans[vertex].x = alpha + alpha*object->vlist_trans[vertex].x;
		object->vlist_trans[vertex].y = beta + beta*object->vlist_trans[vertex].y;
	}
}

void Perspective_To_Screen_RenderList4D(RenderList4D_PTR render_list, Camera4D_PTR camera)
{
	float alpha = 0.5f*camera->viewport_width - 0.5f;
	float beta = 0.5f*camera->viewport_height - 0.5f;

	for (int poly = 0; poly < render_list->num_polys; poly++)
	{
		Poly4D_PTR curr_poly = render_list->poly_ptrs[poly];
		if (curr_poly == NULL || !(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_CLIPPED || curr_poly->state & POLY4D_STATE_BACKFACE)
			continue;
		for (int vertex = 0; vertex < 3; vertex++)
		{
			curr_poly->tverts[vertex].x = alpha + alpha * curr_poly->tverts[vertex].x;
			curr_poly->tverts[vertex].y = beta + beta * curr_poly->tverts[vertex].y;
		}
	}
}

void Build_Perspective_To_Screen_Matrix4X4(Camera4D_PTR camera, Matrix4X4_PTR matrix)
{
	float alpha = 0.5f*camera->viewplane_width - 0.5f;
	float beta = 0.5f*camera->viewplane_height - 0.5f;

	Mat_Init_4X4(matrix, alpha, 0, 0, 0,
		0, -beta, 0, 0,
		alpha, beta, 1, 0,
		0, 0, 0, 1);
}

