#include "light.h"


MAT materials[MAX_MATERIALS];		//ϵͳ�еĲ���
int num_materials;					//��ǰ�Ĳ�����
Light lights[MAX_LIGHTS];			//��Դ����
int num_lights;						//��ǰ�Ĺ�Դ��

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
		//���ܲ����Ƿ��ڻ״̬�����ͷ���֮�����������ͼ
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
	RGBA _c_ambient, RGBA _c_diffuse, RGBA _c_specular,		//��ǿ��
	Point4D_PTR _pos, Vector4D_PTR _dir,
	float _kc, float _kl, float _kq,						//˥������
	float _spot_inner, float _spot_outer,					//�۹����/��׶��
	float _pf)
{
	if (index < 0 || index >= MAX_LIGHTS) //����ֵ�����˹�Դ����ķ�Χ
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
	unsigned int r_base, g_base, b_base,	//ԭ������ɫֵ
		r_sum, g_sum, b_sum,				//ȫ����Դ���������Ч��
		shaded_color;						//������ɫ

	float dp,		//���
		dist,		//����͹�Դ֮��ľ���
		i,			//ǿ��
		nl,			//���߳���
		atten;		//˥��������

	for (int poly = 0; poly < renderlist->num_polys; poly++)
	{
		Poly4D_PTR curr_poly = renderlist->poly_ptrs[poly];

		if (!(curr_poly->state & POLY4D_STATE_ACTIVE) | curr_poly->state &POLY4D_STATE_BACKFACE | curr_poly->state & POLY4D_STATE_CLIPPED)
			continue;


		//int vindex0 = curr_poly->tverts[0];
		//������ε���ɫģʽ
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

				if (lights[curr_light].attr & LIGHT_ATTR_AMBIENT) //������
				{
					//��ÿ�����������ε���ɫ��ˣ�������256����ȷ�����Ϊ0-256֮��
					r_sum += ((lights[curr_light].c_ambient.r * r_base) / 256);
					g_sum += ((lights[curr_light].c_ambient.g * g_base) / 256);
					b_sum += ((lights[curr_light].c_ambient.b * b_base) / 256);
				}
				else if (lights[curr_light].attr & LIGHT_ATTR_INFINITE)
				{
					//����Զ��Դ����Ҫ֪���淨�ߺ͹�Դ����
					Vector4D u, v, n;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					//���������
					Vector4D_Cross(&v, &u, &n);
					
					//��һ����Ϊ���Ż�������裬����Ԥ�ȼ�������ߵĳ���
					nl = Vector4D_Length(&n);

					//����Զ��Դ�Ĺ���ģ�ͣ�
					//I(d) dir = IOdir * Cldir
					//ɢ����ļ��㣺
					//Itotal = Rsdiffuse*(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);
					if (dp > 0)
					{
						//����128��Ϊ�˱��⸡�����㣬������Ϊ���������������Ǹ�����ת���������Ĵ���CPU����
						i = 128 * dp / nl;
						r_sum += (lights[curr_light].c_diffuse.r * r_base * i) / (256 * 128);
						g_sum += (lights[curr_light].c_diffuse.g * g_base * i) / (256 * 128);
						b_sum += (lights[curr_light].c_diffuse.b * b_base * i) / (256 * 128);
					}
					
				}
				else if (lights[curr_light].attr & LIGHT_ATTR_POINT) //���Դ
				{
					//���Դ�Ĺ���ģ�ͣ�
					//                IOpoint * Clpoint
					// I(d)point = ������������������������������������������������
					//              kc + kl * d + kq * d^2
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&u, &v, &n);

					//�Ż����㣬Ԥ�ȵó�����η��ߵĳ���
					nl = Vector4D_Length(&n);

					//����ӱ��浽��Դ������
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//��������˥��
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
					//ִ�о۹�ƹ��ռ��㣬����ʹ�ü򻯵�ģ�ͣ��ô�����ĵ��Դ��ģ��۹��
					//                IOpoint * Clpoint 
					//  I(d)point = ����������������������������������������������������������������
					//                  kc + kl*d + kq*d^2
					// ���� d = |p - s|
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//�Ż�����
					nl = Vector4D_Length(&n);

					//����ӱ��浽��Դ������
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//��������˥��
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
					//�򻯵ľ۹�ƹ���ģ�ͣ�
					//                 IOspotlight * Clspotlight * MAX( (l . s), 0)^pf
					//I(d)spotlight = ����������������������������������������������������������������������������������������������������
					//                       kc + kl * d + kq * d^2
					//���� d = |p -s| , pf=ָ������
					//���˷����а����ڹ�Դ���������ԽǶ���ص�����
					//�⼸������Դģ����ͬ

					Vector4D u, v, n, d, s;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//Ԥ�������η��ߵĳ��ȣ������Ż�
					nl = Vector4D_Length(&n);

					//����ɢ���
					// Itotal = Rsdiffuse * Idiffuse *(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);

					if (dp > 0)
					{
						//����ӱ��浽��Դ������
						Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &s);

						dist = Vector4D_Length(&s);

						//�������� (s . l)
						float dpsl = Vector4D_Dot(&s, &lights[curr_light].dir) / dist;
						
						//��������Ϊ���Ž�����������
						if (dpsl > 0)
						{
							//����˥��
							atten = (lights[curr_light].kc + lights[curr_light].kl * dist + lights[curr_light].kq * dist * dist);

							//��Ϊ����ٶȣ�pf�����Ǵ���1.0������
							float dpsl_exp = dpsl;

							//���������η�
							for (int e_index = 1; e_index < (int)lights[curr_light].pf; e_index++)
							{
								dpsl_exp *= dpsl;
							}
							//����dpsl_exp �洢����dpsl ^ pf, ��(s . l) ^ pf

							i = 128 * dpsl_exp / (nl * atten);

							r_sum += (lights[curr_light].c_diffuse.r * r_base *i) / (256 * 128);
							g_sum += (lights[curr_light].c_diffuse.g * g_base *i) / (256 * 128);
							b_sum += (lights[curr_light].c_diffuse.b * b_base *i) / (256 * 128);

						}
					}
				}
			}

			//ȷ����ɫ���������
			if (r_sum > 255) r_sum = 255;
			if (g_sum > 255) g_sum = 255;
			if (b_sum > 255) b_sum = 255;

			shaded_color = RGBA32BIT(r_sum, g_sum, b_sum, 255);//RGB16Bit(r_sum, g_sum, b_sum);

			//д����ɫ
			//curr_poly->color = (int)((shaded_color << 16) | curr_poly->color);
			curr_poly->lit_color[0] = (int)shaded_color;
		}
		else if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_GOURAUD) //����gouraud��ɫģʽ
		{
			//������ɫģʽ��Ҫ����ÿ������Ĺ���

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

				if (lights[curr_light].attr & LIGHT_ATTR_AMBIENT) //������
				{
					//��ÿ�����������ε���ɫ��ˣ�������256����ȷ�����Ϊ0-256֮��
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
					//����Զ��Դ����Ҫ֪���淨�ߺ͹�Դ����
					Vector4D u, v, n;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					//��������ߡ���Ϊ����û�ж����������ν��м��㣬�������еĶ�����Ȼ�����淨�ߣ�����ܹ��������ڵ������Σ���ҪΪÿ�����㵥�����㷨�ߣ�
					Vector4D_Cross(&v, &u, &n);

					//��һ����Ϊ���Ż�������裬����Ԥ�ȼ�������ߵĳ���
					nl = Vector4D_Length(&n);

					//����Զ��Դ�Ĺ���ģ�ͣ�
					//I(d) dir = IOdir * Cldir
					//ɢ����ļ��㣺
					//Itotal = Rsdiffuse*(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);
					if (dp > 0)
					{
						//����128��Ϊ�˱��⸡�����㣬������Ϊ���������������Ǹ�����ת���������Ĵ���CPU����
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
				else if (lights[curr_light].attr & LIGHT_ATTR_POINT) //���Դ
				{
					//���Դ�Ĺ���ģ�ͣ�
					//                IOpoint * Clpoint
					// I(d)point = ������������������������������������������������
					//              kc + kl * d + kq * d^2
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&u, &v, &n);

					//�Ż����㣬Ԥ�ȵó�����η��ߵĳ���
					nl = Vector4D_Length(&n);

					//����ӱ��浽��Դ������
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//��������˥��
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
					//ִ�о۹�ƹ��ռ��㣬����ʹ�ü򻯵�ģ�ͣ��ô�����ĵ��Դ��ģ��۹��
					//                IOpoint * Clpoint 
					//  I(d)point = ����������������������������������������������������������������
					//                  kc + kl*d + kq*d^2
					// ���� d = |p - s|
					Vector4D u, v, n, l;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//�Ż�����
					nl = Vector4D_Length(&n);

					//����ӱ��浽��Դ������
					Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &l);

					//��������˥��
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
					//�򻯵ľ۹�ƹ���ģ�ͣ�
					//                 IOspotlight * Clspotlight * MAX( (l . s), 0)^pf
					//I(d)spotlight = ����������������������������������������������������������������������������������������������������
					//                       kc + kl * d + kq * d^2
					//���� d = |p -s| , pf=ָ������
					//���˷����а����ڹ�Դ���������ԽǶ���ص�����
					//�⼸������Դģ����ͬ

					Vector4D u, v, n, d, s;
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[1].v, &u);
					Vector4D_Build(&curr_poly->tverts[0].v, &curr_poly->tverts[2].v, &v);

					Vector4D_Cross(&v, &u, &n);

					//Ԥ�������η��ߵĳ��ȣ������Ż�
					nl = Vector4D_Length(&n);

					//����ɢ���
					// Itotal = Rsdiffuse * Idiffuse *(n . l)
					dp = Vector4D_Dot(&n, &lights[curr_light].dir);

					if (dp > 0)
					{
						//����ӱ��浽��Դ������
						Vector4D_Build(&curr_poly->tverts[0].v, &lights[curr_light].pos, &s);

						dist = Vector4D_Length(&s);

						//�������� (s . l)
						float dpsl = Vector4D_Dot(&s, &lights[curr_light].dir) / dist;

						//��������Ϊ���Ž�����������
						if (dpsl > 0)
						{
							//����˥��
							atten = (lights[curr_light].kc + lights[curr_light].kl * dist + lights[curr_light].kq * dist * dist);

							//��Ϊ����ٶȣ�pf�����Ǵ���1.0������
							float dpsl_exp = dpsl;

							//���������η�
							for (int e_index = 1; e_index < (int)lights[curr_light].pf; e_index++)
							{
								dpsl_exp *= dpsl;
							}
							//����dpsl_exp �洢����dpsl ^ pf, ��(s . l) ^ pf

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
		else //���ù̶���ɫ����ԭ������ɫ���Ƶ�ǰ16λ��
		{
			//curr_poly->color = (int)((curr_poly->color << 16) | curr_poly->color);
			curr_poly->lit_color[0] = curr_poly->vert_color[0];
		}
	}
	return 1;
}

void Transform_Lights(Light_PTR lights, //Ҫ�任�Ĺ�Դ����
	int num_lights,					   //Ҫ�任�Ĺ�Դ����
	Matrix4X4_PTR matrix,			   //�任����
	int coor_select)
{
	int curr_lights;
	Matrix4X4 mt;		//���ڴ洢����ƽ�Ʋ��ֵľ���

	if (matrix != NULL)
	{
		Mat_Copy_4X4(matrix, &mt);

		//��Ϊ�Ǳ任��Դ���򣬲���Ҫ�����е�ƽ�Ʋ���
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
