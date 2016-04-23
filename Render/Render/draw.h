#ifndef H_DRAW
#define H_DRAW

//#include <Windows.h>
#include "poly.h"
#include "math3d.h"
#include "light.h"

#define TRI_TYPE_NONE			0
#define TRI_TYPE_FLAT_TOP		1
#define TRI_TYPE_FLAT_BOTTOM	2
#define TRI_TYPE_FLAT_MASK		3
#define TRI_TYPE_GENERAL		4
#define INTERP_LHS				0
#define INTERP_RHS				1

#define FIXP16_SHIFT			16
#define FIXP16_ROUND_UP			0x00008000

#define SWAP(x1,x2,tmp) {tmp = x1; x1 = x2; x2 = tmp;}

int Draw_Line(int x0, int y0, int x1, int y1, int color, unsigned char *vb_start, int lpitch);
int Draw_Pixel(int x, int y, int color, unsigned char *video_buffer, int lpitch);
int Draw_Clip_Line(int x0, int y0, int x1, int y1, int color, unsigned char *dest_buffer, int lpitch);
void Draw_RenderList4D_Wire(RenderList4D_PTR render_list, unsigned char *video_buffer, int lpitch);
void Draw_RenderList4D_Solid(RenderList4D_PTR render_list, unsigned char *video_buffer, int lpitch, unsigned char *zbuffer, int zpitch);
void Draw_Gouraud_Triangle(Poly4D_PTR poly, unsigned char *_dest_buffer, int mempitch);
void Draw_Gouraud_Triangle_Float(Poly4D_PTR poly, UCHAR *_dest_buffer, int mempitch, UCHAR* _z_buffer, int zpitch);
void Draw_Textured_Triangle(Poly4D_PTR poly, unsigned char *_dest_buffer, int mempitch, unsigned char *zbuffer, int zpitch);

void Draw_Triangle_2D(int x1, int y1, int x2, int y2, int x3, int y3, int color, unsigned char * dest_buffer, int mempitch);

#endif
