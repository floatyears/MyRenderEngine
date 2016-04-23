#ifndef H_POLY
#define H_POLY

//#include <Windows.h>
#include <stdio.h>
#include <string>
#include <QtDebug>

//坐标变换
#define TRANSFORM_LOCAL_ONLY			1
#define TRANSFORM_TRANS_ONLY			2
#define TRANSFORM_LOCAL_TO_TRANS		3
#define TRANSFORM_COPY_LOCAL_TO_TRANS	4

//多边形的状态值
#define POLY4D_STATE_ACTIVE				0x0001
#define POLY4D_STATE_VISIBLE			0x0002
#define POLY4D_STATE_BACKFACE			0x0004
#define POLY4D_STATE_CLIPPED			0x0008

//多边形的属性
#define POLY4D_ATTR_2SIDED				0x0001
#define POLY4D_ATTR_TRANSPARENT			0x0002
#define POLY4D_ATTR_8BITCOLOR			0x0004
#define POLY4D_ATTR_RGB16				0x0008
#define POLY4D_ATTR_RGB24				0x0010

//物体的状态
#define OBJECT4D_STATE_ACTIVE			0x0001
#define OBJECT4D_STATE_VISIBLE			0x0002
#define OBJECT4D_STATE_CULLED			0x0004

#define CULL_OBJECT_X_PLANE				0x0001
#define CULL_OBJECT_Y_PLANE				0x0002
#define CULL_OBJECT_Z_PLANE				0x0004

//3D裁剪
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

//顶点定义
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
			//顶点
			Point4D v;
			//法线
			Vector4D n;
			//纹理坐标
			Point2D t;
		};
	};
}Vertex4D, *Vertex4D_PTR;

typedef struct Bitmap_Image_TYP
{
	int state;
	int attr;
	int x, y;			//位图的位置
	int width, height;  //位图的长宽
	int num_bytes;		//位图的总位长
	int bpp;			//每像素的位长
    unsigned char *buffer;		//位图的像素
}Bitmap_Image, *Bitmap_Iamge_PTR;

//自包含数据的多边形定义
typedef struct Poly4D_TYP
{
	int state;					//状态（激活、被剔除、
	int attr;					//着色模式
	//int color;					//颜色
	int lit_color[3];			//用于存储光照处理之后的颜色
	int vert_color[3];			//存储顶点的初始颜色

	Vertex4D verts[3];			//原始点
	Vertex4D tverts[3];			//变换后的点

	Poly4D_TYP *prev;			//指向渲染列表中的上一个多边形的指针
	Poly4D_TYP *next;			//指向渲染列表中的下一个多边形的指针

	Bitmap_Iamge_PTR texture;	//纹理

	Vertex4D normal;			//法线
	float nlength;				//法线长度

}Poly4D, *Poly4D_PTR;

//引用外包数据的多边形定义，主要用于索引三角形
typedef struct PolyF4D_TYP
{
	int state;
	int attr;
	int color;
	int lit_color[3];

	Vertex4D_PTR vlist;			//顶点列表
	int vert[3];				//顶点列表中的元素索引
}PolyF4D, *PolyF4D_PTR;

typedef struct Object4D_TYP
{
	int id;
	char name[64];
	int attr;
	int state;

	Point4D world_pos;
	Vector4D dir;				//物体在局部坐标系中的朝向
	Vector4D ux, uy, uz;		//记录物体朝向的局部坐标轴，物体旋转时，将相应的旋转

	float max_radius;			//最大半径
	float avg_radius;			//平均半径
	
	int num_vertices;			//物体的顶点数
	Vertex4D vlist_local[OBJECT4D_MAX_VERTICES];	//存储局部坐标的数组
	Vertex4D vlist_trans[OBJECT4D_MAX_VERTICES];	//存储变换后的坐标数组

	int num_polys;								//物体网格的多边形数量
	PolyF4D plist[OBJECT4D_MAX_POLYS];			//指向多边形的指针，这里多边形中的顶点是指向物体中的顶点列表的，自己并不包含数据
}Object4D, *Object4D_PTR;

typedef struct RenderList4D_TYP
{
	int state;
	int attr;
	//指针数组，用来排序或者其他的操作
	Poly4D_PTR poly_ptrs[RENDERLIST4D_MAX_POLYS];
	//原始数据，为了避免每帧为多边形分配和释放存储空间，存储在这里
	Poly4D poly_data[RENDERLIST4D_MAX_POLYS];

	int num_polys;				//渲染列表中的多边形数目
}RenderList4D, *RenderList4D_PTR;

typedef struct Plane3D_TYP
{
	Point3D p0;		//平面上的一个点
	Vector3D n;		//平面的法线
}Plane3D, *Plane3D_PTR;

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

	float view_dist;

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

typedef struct ZBUFFER_TYP
{
	int attr;
    unsigned char *zbuffer;
	int width;
	int height;
	int sizeq;			//大小，单位为四元组
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
