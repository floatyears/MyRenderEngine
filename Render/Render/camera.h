#ifndef H_CAMERA
#define H_CAMERA

#define CAM_ROT_SEQ_XYZ  0
#define CAM_ROT_SEQ_YXZ  1
#define CAM_ROT_SEQ_XZY  2
#define CAM_ROT_SEQ_YZX  3
#define CAM_ROT_SEQ_ZYX  4
#define CAM_ROT_SEQ_ZXY  5

#define UVN_MODE_SPHERICAL	0

#define CAM_PROJ_NORMALIZED			0x0001
#define CAM_PROJ_SCREEN				0x0002
#define CAM_PROJ_FOV90				0x0004

#define CAM_MODEL_EULER				0x0008
#define CAM_MODEL_UVN				0x0010




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


void Build_Camera4D_Matrix_Euler(Camera4D_PTR camera, int camera_rotation_seq);
void Build_Camera4D_Matrix_UVN(Camera4D_PTR camera, int mode);
void Camera_To_Perspective_Object4D(Object4D_PTR object, Camera4D_PTR camera);
void Convert_From_Homogeneous4D_Object4D(Object4D_PTR object);
void Camera_To_Perspective_RenderList4D(RenderList4D_PTR render_list, Camera4D_PTR camera);
void Convert_From_Homogeneous4D_RenderList4D(RenderList4D_PTR render_list);
void Build_Camera_To_Perspective_Matrix4X4(Camera4D_PTR camera, Matrix4X4_PTR matrix);
void Perspective_To_Screen_Object4D(Object4D_PTR object, Camera4D_PTR camera);
void Perspective_To_Screen_RenderList4D(RenderList4D_PTR render_list, Camera4D_PTR camera);
void Perspective_To_Screen_RenderList4D(RenderList4D_PTR render_list, Camera4D_PTR camera);
void Build_Perspective_To_Screen_Matrix4X4(Camera4D_PTR camera, Matrix4X4_PTR matrix);

#endif