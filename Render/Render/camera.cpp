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
	Vector4D_COPY(camera->pos, cam_pos);
	Vector4D_COPY(camera->dir, cam_dir);

	//对于uvn相机
	Vector4D_InitXYZ(camera->u, 1, 0, 0);
	Vector4D_InitXYZ(camera->v, 0, 1, 0);
	Vector4D_InitXYZ(camera->n, 0, 0, 1);

	if (cam_target != NULL) //uvn目标位置
		Vector4D_COPY(camera->target, cam_target);
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

	//float tan_fov_div2 = tan(Deg_to)
}