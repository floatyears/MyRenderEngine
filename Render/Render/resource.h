#ifndef RESOURCE_H
#define RESOURCE_H

//#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdio.h>
//#include <Windows.h>
//#include "draw_windows.h"
#include "draw_qt.h"
#include "poly.h"

#define BITMAP_ID            0x4D42 // universal id for a bitmap
#define MAX_COLORS_PALETTE   256
#define BITMAP_STATE_DEAD    0
#define BITMAP_STATE_ALIVE   1
#define BITMAP_STATE_DYING   2 
#define BITMAP_ATTR_LOADED   128


#define BITMAP_EXTRACT_MODE_CELL  0
#define BITMAP_EXTRACT_MODE_ABS   1

//这里会出现结构体内存对齐问题，读取的时候会导致字节数不匹配
typedef struct tagBITMAPFILEHEADER {
  WORD  bfType;
  DWORD bfSize;
  WORD  bfReserved1;
  WORD  bfReserved2;
  DWORD bfOffBits;
//  WORD  bfReserved2;
//  WORD  bfReserved1;
//  DWORD bfSize;
//  WORD  bfType;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
  DWORD biSize;
  LONG  biWidth;
  LONG  biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG  biXPelsPerMeter;
  LONG  biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct Bitmap_File_Tag
{
	BITMAPFILEHEADER	bitmapfileheader;
	BITMAPINFOHEADER	bitmapinfoheader;
    //PALETTEENTRY		palette[256];
	UCHAR				*buffer;			//指向数据
}Bitmap_File, *Bitmap_File_PTR;

int Load_Bitmap_File_Qt(Bitmap_File_PTR bitmap, char *filename);
int Load_Bitmap_File(Bitmap_File_PTR bitmap, char *filename);
int Create_Bitmap(Bitmap_Iamge_PTR image, int x, int y, int width, int height, int bpp);
int Load_Image_Bitmap(Bitmap_Iamge_PTR image, //需要加载数据的图片
	Bitmap_File_PTR bitmap, //被扫描的图片
	int cx, int cy, // 以 单位坐标还是绝对坐标 来扫描图片
	int mode);
int Unload_Bitmap_File(Bitmap_File_PTR bitmap);

#endif
