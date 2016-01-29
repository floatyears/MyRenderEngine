#ifndef H_CAMERA
#define H_CAMERA

#include "math.h"
#include "poly.h"

#define CAM_ROT_SEQ_XYZ  0
#define CAM_ROT_SEQ_YXZ  1
#define CAM_ROT_SEQ_XZY  2
#define CAM_ROT_SEQ_YZX  3
#define CAM_ROT_SEQ_ZYX  4
#define CAM_ROT_SEQ_ZXY  5

#define UVN_MODE_SPHERICAL	0


typedef struct Camera4D_TYP
{
	int state;
	int attr;

	Point4D pos;			//相机在世界坐标系中的位置

	Vector4D dir;			//欧拉角度或者UVN相机模型的注视方向
	
	Vector4D u;				//uvn相机的朝向
	Vector4D v;
	Vector4D n;

	Point4D target;			//uvn模型的目标位置

	float view_dist_h;		//水平视距
	float view_dist_v;		//垂直视距

	float fov;				//水平方向和垂直方向的视野

	float near_clip_z;		//近裁剪面
	float far_clip_z;		//远裁剪面

	Plane3D rt_clip_plane;	//右裁剪面
	Plane3D lt_clip_plane;	//左裁剪面
	Plane3D tp_clip_plane;	//上裁剪面
	Plane3D bt_clip_plane;	//下裁剪面

	float viewplane_width;	//视平面的宽度和高度
	float viewplane_height; //

	float viewport_width;	//视口的大小
	float viewport_height; 
	float viewport_center_x;//视口中心
	float viewport_center_y;

	float aspect_ratio;		//屏幕的宽高比

	Matrix4X4 mcam;			//世界坐标到相机坐标的变换矩阵
	Matrix4X4 mper;			//相机坐标到透视坐标的矩阵
	Matrix4X4 mscr;			//透视坐标到屏幕坐标的矩阵
}Camera4D, *Camera4D_PTR;

void Init_Camera4D(Camera4D_PTR camera,
	int cam_attr,
	Point4D_PTR cam_pos,
	Vector4D_PTR cam_dir,
	Point4D_PTR cam_target, //uvn相机的初始目标位置
	float near_clip_z,
	float far_clip_z,
	float fov,
	float viewport_width,
	float viewport_height);

#endif