#include "light.h"


MAT materials[MAX_MATERIALS];		//系统中的材质
int num_materials;					//当前的材质数
Light lights[MAX_LIGHTS];			//光源数组
int num_lights;						//当前的光源数

int Reset_Materials_Mat(void)
{
	static int first_time = 1;
    if (first_time)
    {
		memset(materials, 0, MAX_MATERIALS*sizeof(MAT));
		first_time = 0;
	}

	for (int curr_matt = 0; curr_matt < MAX_MATERIALS; curr_matt++)
	{
		//不管材质是否处于活动状态，都释放与之相关联的纹理图
		Destroy_Bitmap(&materials[curr_matt].texture);
		memset(&materials[curr_matt], 0, sizeof(MAT));
	}

	return 1;
}

int Reset_Lights_Light(void)
{
	static int first_time = 1;
	memset(lights, 0, MAX_LIGHTS*sizeof(Light));

	num_lights = 0;

	first_time = 1;

	return 1;
}

void Destroy_Bitmap(BITMAP *texture)
{

}

int Init_Lights_Light(int index,
	int _state,
	int _attr,
	RGBA _c_ambient, RGBA _c_diffuse, RGBA _c_specular,		//光强度
	Point4D_PTR _pos, Vector4D_PTR _dir,
	float _kc, float _kl, float _kq,						//衰减因子
	float _spot_inner, float _spot_outer,					//聚光灯内/外锥角
	float _pf)
{
	if (index < 0 || index >= MAX_LIGHTS) //索引值超过了光源数组的范围
	{
		return 0;
	}

	lights[index].attr = _attr;
	lights[index].id = index;
	lights[index].state = _state;
	lights[index].c_ambient = _c_ambient;
	lights[index].c_diffuse = _c_diffuse;
	lights[index].c_specular = _c_specular;
	lights[index].kc = _kc;
	lights[index].kl = _kl;
	lights[index].kq = _kq;

	if (_pos)
		Vector4D_Copy(&lights[index].pos, _pos);
	if (_dir)
	{
		Vector4D_Copy(&lights[index].dir, _dir);
		Vector4D_Normalize(&lights[index].dir);
	}
		
	lights[index].spot_inner = _spot_inner;
	lights[index].spot_outer = _spot_outer;
	lights[index].pf = _pf;

	return index;
}

int Light_RenderList4D_World(RenderList4D_PTR renderlist, Camera4D_PTR cam, Light_PTR lights, int max_lights)
{
	unsigned int r_base, g_base, b_base,	//原来的颜色值
		r_sum, g_sum, b_sum,				//全部光源的总体光照效果
		shaded_color;						//最后的颜色

	float dp,		//点积
		dist,		//表面和光源之间的距离
		i,			//强度
		nl,			//法线长度
		atten;		//衰减计算结果

	for (int poly = 0; poly < renderlist->num_polys; poly++)
	{
		Poly4D_PTR curr_poly = renderlist->poly_ptrs[poly];

		if (!(curr_poly->state & POLY4D_STATE_ACTIVE) | curr_poly->state &POLY4D_STATE_BACKFACE | curr_poly->state & POLY4D_STATE_CLIPPED)
			continue;


		//int vindex0 = curr_poly->tverts[0];
		//检查多边形的着色模式
		if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_FLAT) //
		{
			_RGBFROM32BIT(curr_poly->vert_color[0], &r_base, &g_base, &b_base);
			//if (dd_pixel_format == DD_PIXEL_FORMAT565)
			//{
				//_RGB565FROM16BIT(curr_poly->color, &r_base, &g_base, &b_base);
				//r_base <<= 3;
				//g_base <<= 2;
				//b_base <<= 3;
			//}
			//else
			//{

			//	//_RGB555FROM16BIT(curr_poly->color, &r_base, &g_base, &b_base);
			//	r_base <<= 3;
			//	g_base <<= 3;
			//	b_base <<= 3;
			//}

			r_sum = 0;
			g_sum = 0;
			b_sum = 0;

			for (int curr_light = 0; curr_light < max_lights; curr_light++)
			{
				if (!lights[curr_light].state)
				{

					continue;
				}

				if (lights[curr_light].attr & LIGHT_ATTR_AMBIENT) //环境光
				{
					//将每个分量与多边形的颜色相乘，并处以256，以确保结果为0-256之间
					r_sum += ((lights[curr_light].c_ambient.r * r_base) / 256);
					g_sum += ((lights[curr_light].c_ambient.g * g_base) / 256);
					b_sum += ((lights[curr_light].c_ambient.b * b_base) / 256);
				}
				else if (lights[curr_light].attr & LIGHT_ATTR_INFINITE)
				{
					//无穷远光源，需要知道面法线和光源方向
					Vector4D u, v, n;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					//计算出法线
					Vector4D_Cross(&v, &u, &n);
					
					//归一化，为了优化这个步骤，可以预先计算出法线的长度
					nl = Vector4D_Length(&n);

					//无穷远光源的光照模型：
					//I(d) dir = IOdir * Cldir
					//散射项的计算：
					//Itotal = Rsdiffuse*(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);
					if (dp > 0)
					{
						//乘以128是为了避免浮点运算，不是因为浮点运算慢，而是浮点数转换可能消耗大量CPU周期
						i = 128 * dp / nl;
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);
					}
					
				}
				else if (lights[curr_light].attr & LIGHT_ATTR_POINT) //点光源
				{
					//点光源的光照模型：
					//                IOpoint * Clpoint
					// I(d)point = ————————————————————————
					//              kc + kl * d + kq * d^2
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&u, &v, &n);

					//优化计算，预先得出多边形法线的长度
					nl = Vector4D_Length(&n);

					//计算从表面到光源的向量
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//计算距离和衰减
					dist = Vector4D_Length(&l);

					dp = Vector4D_Dot(&n, &l);

					if (dp > 0)
					{
						atten = (lights[curr_light].kc + lights[curr_light].kl *dist + lights[curr_light].kq *dist *dist);

						i = 128 * dp / (nl*dist*atten);
						r_sum += (lights[curr_light].c_diffuse.r * r_base*i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base*i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base*i) / (256 * 128);

					}

				}
				else if (lights[curr_light].attr & LIGHT_ATTR_SPOTLIGHT1)
				{
					//执行聚光灯光照计算，这里使用简化的模型：用带方向的点光源来模拟聚光灯
					//                IOpoint * Clpoint 
					//  I(d)point = ————————————————————————————————
					//                  kc + kl*d + kq*d^2
					// 其中 d = |p - s|
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//优化计算
					nl = Vector4D_Length(&n);

					//计算从表面到光源的向量
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//计算距离和衰减
					dist = Vector4D_Length(&l);

					dp = Vector4D_Dot(&n, &l);

					if (dp > 0)
					{
						atten = (lights[curr_light].kc + lights[curr_light].kl *dist + lights[curr_light].kq *dist *dist);

						i = 128 * dp / (nl*dist*atten);
						r_sum += (lights[curr_light].c_diffuse.r * r_base*i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base*i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base*i) / (256 * 128);

					}
				}
				else if (lights[curr_light].attr & LIGHT_ATTR_SPOTLIGHT2)
				{
					//简化的聚光灯光照模型：
					//                 IOspotlight * Clspotlight * MAX( (l . s), 0)^pf
					//I(d)spotlight = ——————————————————————————————————————————————————
					//                       kc + kl * d + kq * d^2
					//其中 d = |p -s| , pf=指数因子
					//出了分子中包含于光源和照射点相对角度相关的项外
					//这几乎与点光源模型相同

					Vector4D u, v, n, d, s;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//预计算多边形法线的长度，用来优化
					nl = Vector4D_Length(&n);

					//对于散射光
					// Itotal = Rsdiffuse * Idiffuse *(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);

					if (dp > 0)
					{
						//计算从表面到光源的向量
						Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &s);

						dist = Vector4D_Length(&s);

						//计算点积项 (s . l)
						float dpsl = Vector4D_Dot(&s, &lights[curr_light].dir) / dist;
						
						//仅当这项为正才进行下述处理
						if (dpsl > 0)
						{
							//计算衰减
							atten = (lights[curr_light].kc + lights[curr_light].kl * dist + lights[curr_light].kq * dist * dist);

							//作为提高速度，pf必须是大于1.0的整数
							float dpsl_exp = dpsl;

							//计算整数次方
							for (int e_index = 1; e_index < (int)lights[curr_light].pf; e_index++)
							{
								dpsl_exp *= dpsl;
							}
							//现在dpsl_exp 存储的是dpsl ^ pf, 即(s . l) ^ pf

							i = 128 * dpsl_exp / (nl * atten);

							r_sum += (lights[curr_light].c_diffuse.r * r_base *i) / (256 * 128);
							g_sum += (lights[curr_light].c_diffuse.g * g_base *i) / (256 * 128);
							b_sum += (lights[curr_light].c_diffuse.b * b_base *i) / (256 * 128);

						}
					}
				}
			}

			//确保颜色分量不溢出
			if (r_sum > 255) r_sum = 255;
			if (g_sum > 255) g_sum = 255;
			if (b_sum > 255) b_sum = 255;

			shaded_color = RGBA32BIT(r_sum, g_sum, b_sum, 255);//RGB16Bit(r_sum, g_sum, b_sum);

			//写入颜色
			//curr_poly->color = (int)((shaded_color << 16) | curr_poly->color);
			curr_poly->lit_color[0] = (int)shaded_color;
		}
		else if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_GOURAUD) //采用gouraud着色模式
		{
			//这种着色模式需要计算每个顶点的光照

			int r_base1, g_base1, b_base1,
				r_base2, g_base2, b_base2;
			int r_sum1, g_sum1, b_sum1,
				r_sum2, g_sum2, b_sum2;
			_RGBFROM32BIT(curr_poly->vert_color[0], &r_base, &g_base, &b_base);
			_RGBFROM32BIT(curr_poly->vert_color[1], &r_base1, &g_base1, &b_base1);
			_RGBFROM32BIT(curr_poly->vert_color[2], &r_base2, &g_base2, &b_base2);

			r_sum = r_sum1 = r_sum2 = 0;
			g_sum = g_sum1 = g_sum2 = 0;
			b_sum = b_sum1 = b_sum2 = 0;

			for (int curr_light = 0; curr_light < max_lights; curr_light++)
			{
				if (!lights[curr_light].state)
				{
                    Light_PTR l = &lights[curr_light];
					continue;
				}

				if (lights[curr_light].attr & LIGHT_ATTR_AMBIENT) //环境光
				{
					//将每个分量与多边形的颜色相乘，并处以256，以确保结果为0-256之间
					r_sum += ((lights[curr_light].c_ambient.r * r_base) / 256);
					g_sum += ((lights[curr_light].c_ambient.g * g_base) / 256);
					b_sum += ((lights[curr_light].c_ambient.b * b_base) / 256);

					r_sum1 += ((lights[curr_light].c_ambient.r * r_base1) / 256);
					g_sum1 += ((lights[curr_light].c_ambient.g * g_base1) / 256);
					b_sum1 += ((lights[curr_light].c_ambient.b * b_base1) / 256);

					r_sum2 += ((lights[curr_light].c_ambient.r * r_base2) / 256);
					g_sum2 += ((lights[curr_light].c_ambient.g * g_base2) / 256);
					b_sum2 += ((lights[curr_light].c_ambient.b * b_base2) / 256);
				}
				else if (lights[curr_light].attr & LIGHT_ATTR_INFINITE)
				{
					//无穷远光源，需要知道面法线和光源方向
					Vector4D u, v, n;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					//计算出法线。因为这里没有对相邻三角形进行计算，所以所有的顶点仍然公用面法线（如果能够计算相邻的三角形，需要为每个顶点单独计算法线）
					Vector4D_Cross(&v, &u, &n);

					//归一化，为了优化这个步骤，可以预先计算出法线的长度
					nl = Vector4D_Length(&n);

					//无穷远光源的光照模型：
					//I(d) dir = IOdir * Cldir
					//散射项的计算：
					//Itotal = Rsdiffuse*(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);
					if (dp > 0)
					{
						//乘以128是为了避免浮点运算，不是因为浮点运算慢，而是浮点数转换可能消耗大量CPU周期
						i = 128 * dp / nl;
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);

						r_sum1 += (lights[curr_light].c_diffuse.r * r_base1 * i) / (256 * 128);
						g_sum1 += (lights[curr_light].c_diffuse.g * g_base1 * i) / (256 * 128);
						b_sum1 += (lights[curr_light].c_diffuse.b * b_base1 * i) / (256 * 128);

						r_sum2 += (lights[curr_light].c_diffuse.r * r_base2 * i) / (256 * 128);
						g_sum2 += (lights[curr_light].c_diffuse.g * g_base2 * i) / (256 * 128);
						b_sum2 += (lights[curr_light].c_diffuse.b * b_base2 * i) / (256 * 128);
					}

				}
				else if (lights[curr_light].attr & LIGHT_ATTR_POINT) //点光源
				{
					//点光源的光照模型：
					//                IOpoint * Clpoint
					// I(d)point = ————————————————————————
					//              kc + kl * d + kq * d^2
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&u, &v, &n);

					//优化计算，预先得出多边形法线的长度
					nl = Vector4D_Length(&n);

					//计算从表面到光源的向量
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//计算距离和衰减
					dist = Vector4D_Length(&l);

					dp = Vector4D_Dot(&n, &l);

					if (dp > 0)
					{
						atten = (lights[curr_light].kc + lights[curr_light].kl *dist + lights[curr_light].kq *dist *dist);

						i = 128 * dp / (nl*dist*atten);
						r_sum += (lights[curr_light].c_diffuse.r * r_base*i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base*i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base*i) / (256 * 128);

						r_sum1 += (lights[curr_light].c_diffuse.r * r_base1*i) / (256 * 128);
						g_sum1 += (lights[curr_light].c_diffuse.g * g_base1*i) / (256 * 128);
						b_sum1 += (lights[curr_light].c_diffuse.b * b_base1*i) / (256 * 128);

						r_sum2 += (lights[curr_light].c_diffuse.r * r_base2*i) / (256 * 128);
						g_sum2 += (lights[curr_light].c_diffuse.g * g_base2*i) / (256 * 128);
						b_sum2 += (lights[curr_light].c_diffuse.b * b_base2*i) / (256 * 128);


					}

				}
				else if (lights[curr_light].attr & LIGHT_ATTR_SPOTLIGHT1)
				{
					//执行聚光灯光照计算，这里使用简化的模型：用带方向的点光源来模拟聚光灯
					//                IOpoint * Clpoint 
					//  I(d)point = ————————————————————————————————
					//                  kc + kl*d + kq*d^2
					// 其中 d = |p - s|
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//优化计算
					nl = Vector4D_Length(&n);

					//计算从表面到光源的向量
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//计算距离和衰减
					dist = Vector4D_Length(&l);

					dp = Vector4D_Dot(&n, &l);

					if (dp > 0)
					{
						atten = (lights[curr_light].kc + lights[curr_light].kl *dist + lights[curr_light].kq *dist *dist);

						i = 128 * dp / (nl*dist*atten);
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);

						r_sum1 += (lights[curr_light].c_diffuse.r * r_base1 * i) / (256 * 128);
						g_sum1 += (lights[curr_light].c_diffuse.g * g_base1 * i) / (256 * 128);
						b_sum1 += (lights[curr_light].c_diffuse.b * b_base1 * i) / (256 * 128);

						r_sum2 += (lights[curr_light].c_diffuse.r * r_base2 * i) / (256 * 128);
						g_sum2 += (lights[curr_light].c_diffuse.g * g_base2 * i) / (256 * 128);
						b_sum2 += (lights[curr_light].c_diffuse.b * b_base2 * i) / (256 * 128);

					}
				}
				else if (lights[curr_light].attr & LIGHT_ATTR_SPOTLIGHT2)
				{
					//简化的聚光灯光照模型：
					//                 IOspotlight * Clspotlight * MAX( (l . s), 0)^pf
					//I(d)spotlight = ——————————————————————————————————————————————————
					//                       kc + kl * d + kq * d^2
					//其中 d = |p -s| , pf=指数因子
					//出了分子中包含于光源和照射点相对角度相关的项外
					//这几乎与点光源模型相同

					Vector4D u, v, n, d, s;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//预计算多边形法线的长度，用来优化
					nl = Vector4D_Length(&n);

					//对于散射光
					// Itotal = Rsdiffuse * Idiffuse *(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);

					if (dp > 0)
					{
						//计算从表面到光源的向量
						Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &s);

						dist = Vector4D_Length(&s);

						//计算点积项 (s . l)
						float dpsl = Vector4D_Dot(&s, &lights[curr_light].dir) / dist;

						//仅当这项为正才进行下述处理
						if (dpsl > 0)
						{
							//计算衰减
							atten = (lights[curr_light].kc + lights[curr_light].kl * dist + lights[curr_light].kq * dist * dist);

							//作为提高速度，pf必须是大于1.0的整数
							float dpsl_exp = dpsl;

							//计算整数次方
							for (int e_index = 1; e_index < (int)lights[curr_light].pf; e_index++)
							{
								dpsl_exp *= dpsl;
							}
							//现在dpsl_exp 存储的是dpsl ^ pf, 即(s . l) ^ pf

							i = 128 * dpsl_exp / (nl * atten);

							r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
							g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
							b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);

							r_sum1 += (lights[curr_light].c_diffuse.r * r_base1 * i) / (256 * 128);
							g_sum1 += (lights[curr_light].c_diffuse.g * g_base1 * i) / (256 * 128);
							b_sum1 += (lights[curr_light].c_diffuse.b * b_base1 * i) / (256 * 128);

							r_sum2 += (lights[curr_light].c_diffuse.r * r_base2 * i) / (256 * 128);
							g_sum2 += (lights[curr_light].c_diffuse.g * g_base2 * i) / (256 * 128);
							b_sum2 += (lights[curr_light].c_diffuse.b * b_base2 * i) / (256 * 128);
						}
					}
				}

                curr_poly->lit_color[0] = RGBA32BIT(r_sum, g_sum, b_sum, 255);
                curr_poly->lit_color[1] = RGBA32BIT(r_sum1, g_sum1, b_sum1, 255);
                curr_poly->lit_color[2] = RGBA32BIT(r_sum2, g_sum2, b_sum2, 255);
			}
		}
		else //采用固定着色，将原来的颜色复制到前16位中
		{
			//curr_poly->color = (int)((curr_poly->color << 16) | curr_poly->color);
			curr_poly->lit_color[0] = curr_poly->vert_color[0];
		}
	}
	return 1;
}

void Transform_Lights(Light_PTR lights, //要变换的光源数组
	int num_lights,					   //要变换的光源数量
	Matrix4X4_PTR matrix,			   //变换矩阵
	int coor_select)
{
	int curr_lights;
	Matrix4X4 mt;		//用于存储消除平移部分的矩阵

	if (matrix != NULL)
	{
		Mat_Copy_4X4(matrix, &mt);

		//因为是变换光源方向，不需要矩阵中的平移部分
		mt.M30 = mt.M31 = mt.M32 = 0;

		switch (coor_select)
		{
		case TRANSFORM_LOCAL_ONLY:
			for ( curr_lights = 0; curr_lights < num_lights; curr_lights++)
			{
				Point4D result;
				Mat_Mul_Vector4D_4X4(&lights[curr_lights].dir, &mt, &result);
				Vector4D_Copy(&lights[curr_lights].dir, &result);
				Mat_Mul_Vector4D_4X4(&lights[curr_lights].pos, matrix, &result);
				Vector4D_Copy(&lights[curr_lights].pos, &result);
			}
			break;
		case TRANSFORM_LOCAL_TO_TRANS:
			for (curr_lights = 0; curr_lights < num_lights; curr_lights++)
			{
				Point4D result;
				Mat_Mul_Vector4D_4X4(&lights[curr_lights].dir, &mt, &lights[curr_lights].tdir);
				Mat_Mul_Vector4D_4X4(&lights[curr_lights].pos, matrix, &lights[curr_lights].tpos);
			}
			break;
		case TRANSFORM_TRANS_ONLY:
			for (curr_lights = 0; curr_lights < num_lights; curr_lights++)
			{
				Point4D result;
				Mat_Mul_Vector4D_4X4(&lights[curr_lights].tdir, &mt, &result);
				Vector4D_Copy(&lights[curr_lights].tdir, &result);
				Mat_Mul_Vector4D_4X4(&lights[curr_lights].tpos, matrix, &result);
				Vector4D_Copy(&lights[curr_lights].tpos, &result);
			}
			break;
		case TRANSFORM_COPY_LOCAL_TO_TRANS:
			for ( curr_lights = 0; curr_lights < num_lights; curr_lights++)
			{
				lights[curr_lights].tdir = lights[curr_lights].dir;
				lights[curr_lights].tpos = lights[curr_lights].tpos;
			}
			break;
		default:
			break;
		}
	}
}
