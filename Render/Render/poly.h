//����任
#define TRANSFORM_LOCAL_ONLY			1
#define TRANSFORM_TRANS_ONLY			2
#define TRANSFORM_LOCAL_TO_TRANS		3

//����ε�״ֵ̬
#define POLY4D_STATE_ACTIVE				0x0001
#define POLY4D_STATE_VISIBLE			0x0002
#define POLY4D_STATE_BACKFACE			0x0004
#define POLY4D_STATE_CLIPPED			0x0008

#define RENDERLIST4D_MAX_POLYS	256

#define OBJECT4D_MAX_VERTICES	128
#define OBJECT4D_MAX_POLYS		64

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
		};
	};
}Vertex4D, *Vertex4D_PTR;

//����ζ���
typedef struct Poly4D_TYP
{
	int state;					//״̬��������޳���
	int attr;					//��ɫģʽ
	int color;					//��ɫ
	int lit_color[3];			//���ڴ洢���մ���֮�����ɫ

	Vertex4D verts[3];			//ԭʼ��
	Vertex4D tverts[3];			//�任��ĵ�

	Poly4D *prev;				//ָ����Ⱦ�б��е���һ������ε�ָ��
	Poly4D *next;				//ָ����Ⱦ�б��е���һ������ε�ָ��

	Vertex4D normal;			//����
}Poly4D, *Poly4D_PTR;

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

typedef struct Plane3D
{
	Point4D p0;		//ƽ���ϵ�һ����
	Vector4D n;		//ƽ��ķ���
};