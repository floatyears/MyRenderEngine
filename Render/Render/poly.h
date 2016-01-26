//����任
#define TRANSFORM_LOCAL_ONLY			1
#define TRANSFORM_TRANS_ONLY			2
#define TRANSFORM_LOCAL_TO_TRANS		3

//����ε�״ֵ̬
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

	Vertex4D normal;			//����
}Poly4D, *Poly4D_PTR;


typedef struct Object4D_TYP
{
	int id;
	char name[64];
	int attr;
	int state;

	Point4D world_pos;
	Vector4D dir;				//�����ھֲ�����ϵ�еĳ���
	Vector4D ux, uy, uz;		//�����ھֲ�����ϵ�е���ת
	
	int num_vertices;			//����Ķ�����

	Vertex4D_PTR vlist_local;
	Vector4D_PTR vlist_trans;

	int num_polys;				//��������Ķ��������
	Poly4D_PTR plist;			//ָ�����ε�ָ��

};

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