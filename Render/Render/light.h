#ifndef H_LIGHT
#define H_LIGHT

#include "poly.h"
#include "math3d.h"
#include "draw_windows.h"
#include <Windows.h>

//�����������
#define POLY4D_ATTR_SHADE_MODE_PURE					0x0020
#define POLY4D_ATTR_SHADE_MODE_CONSTANT				0x0020	//����
#define POLY4D_ATTR_SHADE_MODE_FLAT					0x0040  
#define POLY4D_ATTR_SHADE_MODE_GOURAUD				0x0080
#define POLY4D_ATTR_SHADE_MODE_PHONG				0x0100
#define POLY4D_ATTR_SHADE_MODE_FASTPHONG			0x0100 //����
#define POLY4D_ATTR_SHADE_MODE_TEXTURE				0x0200

//�����������
#define MAT_ATTR_2SIDED								0x0001
#define MAT_ATTR_TRANSPARENT						0x0002
#define MAT_ATTR_8BITCOLOR							0x0004
#define MAT_ATTR_RGB16								0x0008
#define MAT_ATTR_RGB24								0x0010

#define MAT_ATTR_SHADE_MODE_CONSTANT				0x0020
#define MAT_ATTR_SHADE_MODE_CONSTANT				0x0020	//����
#define MAT_ATTR_SHADE_MODE_FLAT					0x0040  
#define MAT_ATTR_SHADE_MODE_GOURAUD					0x0080
#define MAT_ATTR_SHADE_MODE_PHONG					0x0100
#define MAT_ATTR_SHADE_MODE_FASTPHONG				0x0100 //����
#define MAT_ATTR_SHADE_MODE_TEXTURE					0x0200

#define MAT_STATE_ACTIVE							0x0001

#define MAX_MATERIALS								256

#define LIGHT_ATTR_AMBIENT							0x0001
#define LIGHT_ATTR_INFINITE							0x0002
#define LIGHT_ATTR_POINT							0x0004
#define LIGHT_ATTR_SPOTLIGHT1						0x0008 //1�ࣨ�򵥣��۹��
#define LIGHT_ATTR_SPOTLIGHT2						0x0010 //2�ࣨ���ӣ��۹��

#define LIGHT_STATE_ON								1
#define LIGHT_STATE_OFF								0

#define MAX_LIGHTS									8

typedef struct RGBA_TYP
{
	union
	{
		int rgba;			//ѹ����ʽ
		UCHAR rgba_M[4];	//�����ʽ
		struct				//��ʽ���Ƹ�ʽ
		{
			UCHAR a, b, g, r;
		};
	};
} RGBA, *RGBA_PTR;

typedef	struct MAT_TYP
{
	int id;
	int state;
	char name[64];
	int attr;

	RGBA color;					//������ɫ
	float ka, kd, ks, power;	//�Ի����⡢ɢ���;��淴���ķ���ϵ���;��淴��ָ��
	RGBA ra, rd, rs;			//Ԥ�ȼ���õ�����ɫ�ͷ���ϵ���Ļ�

	char texture_files[80];		//����������ļ�λ��
	BITMAP texture;				//����ͼ
	
} MAT, *MAT_PTR;

typedef struct Light_TYP
{
	int state;				//��Դ״̬
	int id;					//��Դid
	int attr;				//��Դ���ͼ���������

	RGBA c_ambient;			//������ǿ��
	RGBA c_diffuse;			//ɢ���ǿ��	
	RGBA c_specular;		//���淴���ǿ��

	Point4D pos;			//��Դλ��
	Point4D tpos;			//�任֮��Ĺ�Դλ��
	Vector4D dir;			//��Դ����
	Vector4D tdir;			//�任֮��Ĺ�Դ����

	float kc, kl, kq;		//˥������
	float spot_inner;		//�۹����׶��
	float spot_outer;		//�۹����׶��

	float pf;				//�۹��ָ������
}Light, *Light_PTR;

extern MAT materials[MAX_MATERIALS];		//ϵͳ�еĲ���
extern int num_materials;					//��ǰ�Ĳ�����
extern Light lights[MAX_LIGHTS];			//��Դ����
extern int num_lights;						//��ǰ�Ĺ�Դ��

int Reset_Materials_Mat(void);
int Reset_Lights_Light(void);
int Init_Lights_Light(int index,
	int _state,
	int _attr,
	RGBA _c_ambient, RGBA _c_diffuse, RGBA _c_specular,		//��ǿ��
	Point4D_PTR _pos, Vector4D_PTR _dir,
	float _kc, float _kl, float _kq,						//˥������
	float _spot_inner, float _spot_outer,					//�۹����/��׶��
	float _pf);												//�۹��ָ������
int Light_RenderList4D_World(RenderList4D_PTR renderlist, Camera4D_PTR cam, Light_PTR lights, int max_lights);
//int Init_Materail_Mat(int index, int _state, cha)
void Destroy_Bitmap(BITMAP *texture);

#endif