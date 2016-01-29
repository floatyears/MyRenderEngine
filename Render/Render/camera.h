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

	Point4D pos;			//�������������ϵ�е�λ��

	Vector4D dir;			//ŷ���ǶȻ���UVN���ģ�͵�ע�ӷ���
	
	Vector4D u;				//uvn����ĳ���
	Vector4D v;
	Vector4D n;

	Point4D target;			//uvnģ�͵�Ŀ��λ��

	float view_dist_h;		//ˮƽ�Ӿ�
	float view_dist_v;		//��ֱ�Ӿ�

	float fov;				//ˮƽ����ʹ�ֱ�������Ұ

	float near_clip_z;		//���ü���
	float far_clip_z;		//Զ�ü���

	Plane3D rt_clip_plane;	//�Ҳü���
	Plane3D lt_clip_plane;	//��ü���
	Plane3D tp_clip_plane;	//�ϲü���
	Plane3D bt_clip_plane;	//�²ü���

	float viewplane_width;	//��ƽ��Ŀ�Ⱥ͸߶�
	float viewplane_height; //

	float viewport_width;	//�ӿڵĴ�С
	float viewport_height; 
	float viewport_center_x;//�ӿ�����
	float viewport_center_y;

	float aspect_ratio;		//��Ļ�Ŀ�߱�

	Matrix4X4 mcam;			//�������굽�������ı任����
	Matrix4X4 mper;			//������굽͸������ľ���
	Matrix4X4 mscr;			//͸�����굽��Ļ����ľ���
}Camera4D, *Camera4D_PTR;

void Init_Camera4D(Camera4D_PTR camera,
	int cam_attr,
	Point4D_PTR cam_pos,
	Vector4D_PTR cam_dir,
	Point4D_PTR cam_target, //uvn����ĳ�ʼĿ��λ��
	float near_clip_z,
	float far_clip_z,
	float fov,
	float viewport_width,
	float viewport_height);

#endif