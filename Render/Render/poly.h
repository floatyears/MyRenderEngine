//坐标变换
#define TRANSFORM_LOCAL_ONLY			1
#define TRANSFORM_TRANS_ONLY			2
#define TRANSFORM_LOCAL_TO_TRANS		3

//多边形的状态值
#define POLY4D_STATE_ACTIVE				0x0001
#define POLY4D_STATE_VISIBLE			0x0002
#define POLY4D_STATE_BACKFACE			0x0004
#define POLY4D_STATE_CLIPPED			0x0008

#define RENDERLIST4D_MAX_POLYS 256

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
		};
	};
}Vertex4D, *Vertex4D_PTR;

//多边形定义
typedef struct Poly4D_TYP
{
	int state;					//状态（激活、被剔除、
	int attr;					//着色模式
	int color;					//颜色
	int lit_color[3];			//用于存储光照处理之后的颜色

	Vertex4D verts[3];			//原始点
	Vertex4D tverts[3];			//变换后的点

	Vertex4D normal;			//法线
}Poly4D, *Poly4D_PTR;


typedef struct Object4D_TYP
{
	int id;
	char name[64];
	int attr;
	int state;

	Point4D world_pos;
	Vector4D dir;				//物体在局部坐标系中的朝向
	Vector4D ux, uy, uz;		//物体在局部坐标系中的旋转
	
	int num_vertices;			//物体的顶点数

	Vertex4D_PTR vlist_local;
	Vector4D_PTR vlist_trans;

	int num_polys;				//物体网格的多边形数量
	Poly4D_PTR plist;			//指向多边形的指针

};

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