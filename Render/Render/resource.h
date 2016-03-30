#ifndef RESOURCE_H
#define RESOURCE_H

#include <filesystem>
#include <fstream>
#include <iostream>
#include <io.h>
#include <Windows.h>
#include "draw_windows.h"
#include "poly.h"

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE  256
#define BITMAP_STATE_DEAD    0
#define BITMAP_STATE_ALIVE   1
#define BITMAP_STATE_DYING   2 
#define BITMAP_ATTR_LOADED   128


#define BITMAP_EXTRACT_MODE_CELL  0
#define BITMAP_EXTRACT_MODE_ABS   1

typedef struct Bitmap_File_Tag
{
	BITMAPFILEHEADER	bitmapfileheader;
	BITMAPINFOHEADER	bitmapinfoheader;
	PALETTEENTRY		palette[256];
	UCHAR				*buffer;			//ָ������
}Bitmap_File, *Bitmap_File_PTR;


int Load_Bitmap_File(Bitmap_File_PTR bitmap, char *filename);
int Create_Bitmap(Bitmap_Iamge_PTR image, int x, int y, int width, int height, int bpp);
int Load_Image_Bitmap(Bitmap_Iamge_PTR image, //��Ҫ�������ݵ�ͼƬ
	Bitmap_File_PTR bitmap, //��ɨ���ͼƬ
	int cx, int cy, // �� ��λ���껹�Ǿ������� ��ɨ��ͼƬ
	int mode);
int Unload_Bitmap_File(Bitmap_File_PTR bitmap);

#endif