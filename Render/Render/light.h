#ifndef H_LIGHT
#define H_LIGHT

#include "poly.h"
#include "math3d.h"
#include "draw_windows.h"
#include <Windows.h>

//定义面的属性
#define POLY4D_ATTR_SHADE_MODE_PURE					0x0020
#define POLY4D_ATTR_SHADE_MODE_CONSTANT				0x0020	//别名
#define POLY4D_ATTR_SHADE_MODE_FLAT					0x0040  
#define POLY4D_ATTR_SHADE_MODE_GOURAUD				0x0080
#define POLY4D_ATTR_SHADE_MODE_PHONG				0x0100
#define POLY4D_ATTR_SHADE_MODE_FASTPHONG			0x0100 //别名
#define POLY4D_ATTR_SHADE_MODE_TEXTURE				0x0200

//定义材质属性
#define MAT_ATTR_2SIDED								0x0001
#define MAT_ATTR_TRANSPARENT						0x0002
#define MAT_ATTR_8BITCOLOR							0x0004
#define MAT_ATTR_RGB16								0x0008
#define MAT_ATTR_RGB24								0x0010

#define MAT_ATTR_SHADE_MODE_CONSTANT				0x0020
#define MAT_ATTR_SHADE_MODE_CONSTANT				0x0020	//别名
#define MAT_ATTR_SHADE_MODE_FLAT					0x0040  
#define MAT_ATTR_SHADE_MODE_GOURAUD					0x0080
#define MAT_ATTR_SHADE_MODE_PHONG					0x0100
#define MAT_ATTR_SHADE_MODE_FASTPHONG				0x0100 //别名
#define MAT_ATTR_SHADE_MODE_TEXTURE					0x0200

#define MAT_STATE_ACTIVE							0x0001

#define MAX_MATERIALS								256

#define LIGHT_ATTR_AMBIENT							0x0001
#define LIGHT_ATTR_INFINITE							0x0002
#define LIGHT_ATTR_POINT							0x0004
#define LIGHT_ATTR_SPOTLIGHT1						0x0008 //1类（简单）聚光灯
#define LIGHT_ATTR_SPOTLIGHT2						0x0010 //2类（复杂）聚光灯

#define LIGHT_STATE_ON								1
#define LIGHT_STATE_OFF								0

#define MAX_LIGHTS									8

typedef struct RGBA_TYP
{
	union
	{
		int rgba;			//压缩格式
		UCHAR rgba_M[4];	//数组格式
		struct				//显式名称格式
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

	RGBA color;					//材质颜色
	float ka, kd, ks, power;	//对环境光、散射光和镜面反射光的反射系数和镜面反射指数
	RGBA ra, rd, rs;			//预先计算得到的颜色和反射系数的积

	char texture_files[80];		//包含文理的文件位置
	BITMAP texture;				//纹理图
	
} MAT, *MAT_PTR;

typedef struct Light_TYP
{
	int state;				//光源状态
	int id;					//光源id
	int attr;				//光源类型及其他属性

	RGBA c_ambient;			//环境光强度
	RGBA c_diffuse;			//散射光强度	
	RGBA c_specular;		//镜面反射光强度

	Point4D pos;			//光源位置
	Point4D tpos;			//变换之后的光源位置
	Vector4D dir;			//光源方向
	Vector4D tdir;			//变换之后的光源方向

	float kc, kl, kq;		//衰减因子
	float spot_inner;		//聚光灯内锥角
	float spot_outer;		//聚光灯外锥角

	float pf;				//聚光灯指数因子
}Light, *Light_PTR;

extern MAT materials[MAX_MATERIALS];		//系统中的材质
extern int num_materials;					//当前的材质数
extern Light lights[MAX_LIGHTS];			//光源数组
extern int num_lights;						//当前的光源数

int Reset_Materials_Mat(void);
int Reset_Lights_Light(void);
int Init_Lights_Light(int index,
	int _state,
	int _attr,
	RGBA _c_ambient, RGBA _c_diffuse, RGBA _c_specular,		//光强度
	Point4D_PTR _pos, Vector4D_PTR _dir,
	float _kc, float _kl, float _kq,						//衰减因子
	float _spot_inner, float _spot_outer,					//聚光灯内/外锥角
	float _pf);												//聚光灯指数因子
int Light_RenderList4D_World(RenderList4D_PTR renderlist, Camera4D_PTR cam, Light_PTR lights, int max_lights);
//int Init_Materail_Mat(int index, int _state, cha)
void Destroy_Bitmap(BITMAP *texture);

#endif