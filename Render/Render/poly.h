#ifndef H_POLY
#define H_POLY

//#include <Windows.h>
#include <stdio.h>
#include <string>
#include <QtDebug>

//����任
#define TRANSFORM_LOCAL_ONLY			1
#define TRANSFORM_TRANS_ONLY			2
#define TRANSFORM_LOCAL_TO_TRANS		3
#define TRANSFORM_COPY_LOCAL_TO_TRANS	4

//����ε�״ֵ̬
#define POLY4D_STATE_ACTIVE				0x0001
#define POLY4D_STATE_VISIBLE			0x0002
#define POLY4D_STATE_BACKFACE			0x0004
#define POLY4D_STATE_CLIPPED			0x0008

//����ε�����
#define POLY4D_ATTR_2SIDED				0x0001
#define POLY4D_ATTR_TRANSPARENT			0x0002
#define POLY4D_ATTR_8BITCOLOR			0x0004
#define POLY4D_ATTR_RGB16				0x0008
#define POLY4D_ATTR_RGB24				0x0010

//�����״̬
#define OBJECT4D_STATE_ACTIVE			0x0001
#define OBJECT4D_STATE_VISIBLE			0x0002
#define OBJECT4D_STATE_CULLED			0x0004

#define CULL_OBJECT_X_PLANE				0x0001
#define CULL_OBJECT_Y_PLANE				0x0002
#define CULL_OBJECT_Z_PLANE				0x0004

//3D�ü�
#define CLIP_POLY_X_PLANE				0x0001
#define CLIP_POLY_Y_PLANE				0x0002
#define CLIP_POLY_Z_PLANE				0x0004

#define ZBUFFER_ATTR_16BIT				1
#define ZBUFFER_ATTR_32BIT				2

#define RENDERLIST4D_MAX_POLYS	25600

#define OBJECT4D_MAX_VERTICES	128
#define OBJECT4D_MAX_POLYS		64

#define UCHAR unsigned char
#define UINT unsigned int
#define DWORD unsigned int
#define WORD unsigned short
#define LONG int
#define USHORT unsigned short
#define INT int

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define AMBIENT_LIGHT_INDEX 0
#define INFINITE_LIGHT_INDEX 1

typedef struct Matrix4X4_TYP
{
	union
	{
		float M[4][4];

		struct
		{
			float M00, M01, M02, M03;
			float M10, M11, M12, M13;
			float M20, M21, M22, M23;
			float M30, M31, M32, M33;
		};
	};
} Matrix4X4, *Matrix4X4_PTR;

typedef struct Vector4D_TYP
{
	union
	{
		float M[4];
		struct 
		{
			float x;
			float y;
			float z;
			float w;
		};
	};
}Vector4D, *Vector4D_PTR, Point4D, *Point4D_PTR;

typedef struct Vector2D_TYP
{
	union
	{
		float M[2];
		struct
		{
			float x;
			float y;
		};
	};
}Vector2D, *Vector2D_PTR, Point2D, *Point2D_PTR;

typedef struct Vector3D_TYP
{
	union
	{
		float M[3];
		struct
		{
			float x;
			float y;
			float z;
		};
	};
}Vector3D, *Vector3D_PTR, Point3D, *Point3D_PTR;

//���㶨��
typedef struct Vertex4D_TYP
{
	union
	{
		float M[4];
		struct
		{
			float x, y, z, w;
			float nx, ny, nz, nw;
			float u0, v0;
		};

		struct 
		{
			//����
			Point4D v;
			//����
			Vector4D n;
			//��������
			Point2D t;
		};
	};
}Vertex4D, *Vertex4D_PTR;

typedef struct Bitmap_Image_TYP
{
	int state;
	int attr;
	int x, y;			//λͼ��λ��
	int width, height;  //λͼ�ĳ���
	int num_bytes;		//λͼ����λ��
	int bpp;			//ÿ���ص�λ��
    unsigned char *buffer;		//λͼ������
}Bitmap_Image, *Bitmap_Iamge_PTR;

//�԰������ݵĶ���ζ���
typedef struct Poly4D_TYP
{
	int state;					//״̬��������޳���
	int attr;					//��ɫģʽ
	//int color;					//��ɫ
	int lit_color[3];			//���ڴ洢���մ���֮�����ɫ
	int vert_color[3];			//�洢����ĳ�ʼ��ɫ

	Vertex4D verts[3];			//ԭʼ��
	Vertex4D tverts[3];			//�任��ĵ�

	Poly4D_TYP *prev;			//ָ����Ⱦ�б��е���һ������ε�ָ��
	Poly4D_TYP *next;			//ָ����Ⱦ�б��е���һ������ε�ָ��

	Bitmap_Iamge_PTR texture;	//����

	Vertex4D normal;			//����
	float nlength;				//���߳���

}Poly4D, *Poly4D_PTR;

//����������ݵĶ���ζ��壬��Ҫ��������������
typedef struct PolyF4D_TYP
{
	int state;
	int attr;
	int color;
	int lit_color[3];

	Vertex4D_PTR vlist;			//�����б�
	int vert[3];				//�����б��е�Ԫ������
}PolyF4D, *PolyF4D_PTR;

typedef struct Object4D_TYP
{
	int id;
	char name[64];
	int attr;
	int state;

	Point4D world_pos;
	Vector4D dir;				//�����ھֲ�����ϵ�еĳ���
	Vector4D ux, uy, uz;		//��¼���峯��ľֲ������ᣬ������תʱ������Ӧ����ת

	float max_radius;			//���뾶
	float avg_radius;			//ƽ���뾶
	
	int num_vertices;			//����Ķ�����
	Vertex4D vlist_local[OBJECT4D_MAX_VERTICES];	//�洢�ֲ����������
	Vertex4D vlist_trans[OBJECT4D_MAX_VERTICES];	//�洢�任�����������

	int num_polys;								//��������Ķ��������
	PolyF4D plist[OBJECT4D_MAX_POLYS];			//ָ�����ε�ָ�룬���������еĶ�����ָ�������еĶ����б�ģ��Լ�������������
}Object4D, *Object4D_PTR;

typedef struct RenderList4D_TYP
{
	int state;
	int attr;
	//ָ�����飬����������������Ĳ���
	Poly4D_PTR poly_ptrs[RENDERLIST4D_MAX_POLYS];
	//ԭʼ���ݣ�Ϊ�˱���ÿ֡Ϊ����η�����ͷŴ洢�ռ䣬�洢������
	Poly4D poly_data[RENDERLIST4D_MAX_POLYS];

	int num_polys;				//��Ⱦ�б��еĶ������Ŀ
}RenderList4D, *RenderList4D_PTR;

typedef struct Plane3D_TYP
{
	Point3D p0;		//ƽ���ϵ�һ����
	Vector3D n;		//ƽ��ķ���
}Plane3D, *Plane3D_PTR;

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

	float view_dist;

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

typedef struct ZBUFFER_TYP
{
	int attr;
    unsigned char *zbuffer;
	int width;
	int height;
	int sizeq;			//��С����λΪ��Ԫ��
}ZBUFFER, *ZBUFFER_PTR;

void Remove_Backfaces_Object4D(Object4D_PTR object, Camera4D_PTR camera);
int Cull_Object4D(Object4D_PTR object, Camera4D_PTR camera, int cull_flags);
void Remove_Backfaces_RenderList4D(RenderList4D_PTR render_list, Camera4D_PTR camera);
void Build_Model_To_World_MATRIX4X4(Vector4D_PTR pos, Matrix4X4_PTR m);
void Transform_Object4D(Object4D_PTR object, Matrix4X4_PTR matrix, int coord_select, int transform_basis);
void Transform_RenderList4D(RenderList4D_PTR render_list, Matrix4X4_PTR matrix, int coord_selct);
int Insert_Poly4D_RenderList4D(RenderList4D_PTR render_list, Poly4D_PTR poly);
void Reset_RenderList4D(RenderList4D_PTR render_list);
void Model_To_World_RenderList4D(RenderList4D_PTR rend_list,
	Point4D_PTR world_pos,
	int coord_select = TRANSFORM_LOCAL_TO_TRANS);

void Clip_Poly_RenderList4D(RenderList4D_PTR renderlist, Camera4D_PTR cam, int clip_flags);

int Create_ZBuffer(ZBUFFER_PTR zb, int width, int height, int attr);
int Delete_ZBuffer(ZBUFFER_PTR zb);
void Clear_ZBuffer(ZBUFFER_PTR zb, unsigned int data);

#endif
