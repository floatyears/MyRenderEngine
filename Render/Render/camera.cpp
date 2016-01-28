#include "camera.h"
#include <stddef.h>
#include <math.h>

void Init_Camera4D(Camera4D_PTR camera,
	int cam_attr,
	Point4D_PTR cam_pos,
	Vector4D_PTR cam_dir,
	Point4D_PTR cam_target, //uvn相机的初始目标位置
	float near_clip_z,
	float far_clip_z,
	float fov,
	int viewport_width,
	int viewport_height)
{
	camera->attr = cam_attr;
	Vector4D_Copy(camera->pos, cam_pos);
	Vector4D_Copy(camera->dir, cam_dir);

	//对于uvn相机
	Vector4D_InitXYZ(camera->u, 1, 0, 0);
	Vector4D_InitXYZ(camera->v, 0, 1, 0);
	Vector4D_InitXYZ(camera->n, 0, 0, 1);

	if (cam_target != NULL) //uvn目标位置
		Vector4D_Copy(camera->target, cam_target);
	else
		Vector4D_Zero(camera->target);
	
	camera->near_clip_z = near_clip_z;
	camera->far_clip_z = far_clip_z;

	camera->viewplane_width = viewport_width;
	camera->viewplane_height = viewport_height;
	camera->viewport_center_x = (viewport_width - 1) / 2;
	camera->viewport_center_y = (viewport_height - 1) / 2;

	camera->aspect_ratio = (float)viewport_width / (float)viewport_height;

	//将所有变换矩阵设置为单位矩阵
	Mat_Identity_4X4(&camera->mcam);
	Mat_Identity_4X4(&camera->mper);
	Mat_Identity_4X4(&camera->mscr);

	camera->fov = fov;

	//视平面设置为2x(2/ar)
	camera->viewplane_width = 2.0;
	camera->viewplane_height = 2.0 / camera->aspect_ratio;

	float tan_fov_div2 = tan(DEG_TO_RAD(fov / 2));

	camera->view_dist = 0.5 * camera->viewplane_width * tan_fov_div2;

	if (fov == 90.0)
	{
		Point3D pt_origin; //裁剪面上一个点
		Vector3D_InitXYZ(&pt_origin, 0, 0, 0);

		Vector3D vn; //面法线

		//左裁剪面
		Vector4D_InitXYZ(&vn, 1, 0, -1); //平面x=z
		Plane3D_Init(&camera->rt_clip_plane, &pt_origin, &vn, 1);

		//右裁剪面
		Vector4D_InitXYZ(&vn, -1, 0, -1);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);

		//上裁剪面
		Vector4D_InitXYZ(&vn, 0, 1, -1);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);

		//下裁剪面
		Vector4D_InitXYZ(&vn, 0, -1, -1);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);
	}
	else
	{
		//计算fov不为90度的情况
		Point3D pt_origin; //裁剪面上一个点
		Vector3D_InitXYZ(&pt_origin, 0, 0, 0);

		Vector3D vn;

		Vector3D_InitXYZ(&vn, camera->view_dist, 0, -camera->viewplane_width / 2.0);
		Plane3D_Init(&camera->rt_clip_plane, &pt_origin, &vn, 1);

		Vector3D_InitXYZ(&vn, -camera->view_dist, 0, -camera->viewplane_width / 2.0);
		Plane3D_Init(&camera->lt_clip_plane, &pt_origin, &vn, 1);

		Vector3D_InitXYZ(&vn, 0, camera->view_dist, -camera->viewplane_width / 2.0);
		Plane3D_Init(&camera->tp_clip_plane, &pt_origin, &vn, 1);

		Vector3D_InitXYZ(&vn, 0, -camera->view_dist, -camera->viewplane_width / 2.0);
		Plane3D_Init(&camera->bt_clip_plane, &pt_origin, &vn, 1);
	}
}

void Build_Camera4D_Matrix_Euler(Camera4D_PTR camera, int camera_rotation_seq)
{
	Matrix4X4 mt_inv,		//相机平移矩阵的逆矩阵
		mx_inv,				//相机绕x轴的旋转矩阵的逆矩阵
		my_inv,				//相机绕y轴的旋转矩阵的逆矩阵
		mz_inv,				//相机套z轴的旋转矩阵的逆矩阵
		mrot,				//所有逆旋转矩阵的积
		mtmp;				//用于存储临时矩阵

	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-camera->pos.x, -camera->pos.y, -camera->pos.z, 1);

	//1.提取欧拉角度
	float theta_x = camera->dir.x;
	float theta_y = camera->dir.y;
	float theta_z = camera->dir.z;

	float cos_theta = cos(theta_x);
	float sin_theta = -sin(theta_x);

	//建立矩阵
	Mat_Init_4X4(&mx_inv, 1, 0, 0, 0,
		0, cos_theta, sin_theta, 0,
		0, -sin_theta, cos_theta, 0,
		0, 0, 0, 1);

	cos_theta = cos(theta_y);
	sin_theta = -sin(theta_y);

	//建立矩阵
	Mat_Init_4X4(&my_inv, cos_theta, -sin_theta, 0, 0,
		0, -1, 0, 0,
		sin_theta, 0, cos_theta, 0,
		0, 0, 0, 1);

	cos_theta = cos(theta_z);
	sin_theta = -sin(theta_z);

	//建立矩阵
	Mat_Init_4X4(&my_inv, cos_theta, sin_theta, 0, 0,
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
	Matrix4X4 mt_inv,		//逆相机平移矩阵
		mt_uvn,				//UVN相机变换矩阵
		mtmp;				//用于存储临时矩阵

	Mat_Init_4X4(&mt_inv, 1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		-camera->pos.x, -camera->pos.y, -camera->pos.z, 1);

	//确定如何计算目标点
	if (mode == UVN_MODE_SPHERICAL)
	{
		float phi = camera->dir.x;		//仰角
		float theta = camera->dir.y;	//方位角

		float sin_phi = sin(phi);
		float cos_phi = cos(phi);

		float sin_theta = sin(theta);
		float cos_theta = cos(theta);

		//计算目标点在单位球面上的位置（x,y和z）
		camera->target.x = -1 * sin_phi * sin_theta;
		camera->target.y = 1 * cos_phi;
		camera->target.z = 1 * sin_phi * cos_theta;
	}

	//到这里就有了可以重新计算UVN所需的全部参数：观察参考点和目标点
	//1. n = <目标位置-观察参考点>
	Vector4D_Build(&camera->pos, &camera->target, &camera->n);

	//2. 将v设置为<0,1,0>
	Vector4D_InitXYZ(&camera->v, 0, 1, 0);

	//3. u = (v x n)
	Vector4D_Cross(&camera->v, &camera->n, &camera->u);

	//4. v = (n x u)
	Vector4D_Cross(&camera->n, &camera->u, &camera->v);

	//5. 归一化所有向量
	Vector4D_Normalize(&camera->u);
	Vector4D_Normalize(&camera->v);
	Vector4D_Normalize(&camera->n);

	//将uvn带入，得到UVN旋转矩阵
	Mat_Init_4X4(&mt_uvn, camera->u.x, camera->v.x, camera->n.x, 0,
		camera->u.y, camera->v.y, camera->n.y, 0,
		camera->u.z, camera->v.z, camera->n.z, 0,
		0, 0, 0, 1);

	//将平移矩阵乘以uvn矩阵，
	Mat_Mul_4X4(&mt_inv, &mt_uvn, &camera->mcam);
}