#ifndef H_DRAW
#define H_DRAW

#include <memory.h>
#include <ddraw.h>
#include <stddef.h>
#include <stdio.h>
#include <wingdi.h>

#define DD_PIXEL_FORMAT8        8
#define DD_PIXEL_FORMAT555      15
#define DD_PIXEL_FORMAT565      16
#define DD_PIXEL_FORMAT888      24
#define DD_PIXEL_FORMATALPHA888 32 

#define DDRAW_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		480
#define SCREEN_BPP			8
#define MAX_COLORS_PALETTE  256

HWND main_window_handle = NULL;
HINSTANCE main_instance = NULL;

LPDIRECTDRAWSURFACE7 lpddsprimary = NULL;
LPDIRECTDRAWSURFACE7 lpddsback = NULL;
LPDIRECTDRAWPALETTE  lpddpal = NULL;
LPDIRECTDRAWCLIPPER  lpddclipper = NULL;
LPDIRECTDRAWCLIPPER  lpddcliperwin = NULL;

DDSCAPS2			 ddscaps;
HRESULT				 ddrval;

PALETTEENTRY		 palette[MAX_COLORS_PALETTE];
PALETTEENTRY		 save_palette[MAX_COLORS_PALETTE];

DDSURFACEDESC2 ddsd;
LPDIRECTDRAW7 lpdd = NULL;
UCHAR *primary_buffer = NULL;
UCHAR *back_buffer = NULL;

int dd_pixel_format = DD_PIXEL_FORMAT565;
int primary_lpitch = 0;
int back_lpitch = 0;


int min_clip_x = 0,
	max_clip_x = SCREEN_WIDTH - 1,
	min_clip_y = 0,
	max_clip_y = SCREEN_HEIGHT - 1;

int screen_width = SCREEN_WIDTH,
	screen_height = SCREEN_HEIGHT,
	screen_bpp = SCREEN_BPP,
	screen_windowed = 0;

int window_client_x0 = 0;
int window_client_y0 = 0;

//RGB16µÄº¯ÊýÖ¸Õë
USHORT(*RGB16Bit)(int r, int g, int b) = NULL;

#define _RGB16BIT555(r,g,b) ((b & 31) + (g & 31) << 5 + (r & 31) << 10)
#define _RGB16BIT565(r,g,b) ((b & 31) + (g & 63) << 5 + (r & 31) << 11)
#define _RGB24BIT(r,g,b)	(b + g << 8 + r << 16)
#define _RGB32BIT(a,r,g,b)	(b + g << 8 + r << 16 + a << 24)

USHORT RGB16Bit565(int r, int g, int b);
USHORT RGB16Bit555(int r, int g, int b);
UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds, int *lpitch);
int DDraw_Unlock_Surface(LPDIRECTDRAWSURFACE7 lpdds);
UCHAR *DDraw_Lock_Primary_Surface(void);
int DDraw_Unlock_Primary_Surface(void);
UCHAR *DDraw_Lock_Back_Surface(void);
int DDraw_Unlock_Back_Surface(void);
LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, int mem_flags = 0, USHORT color_key_value = NULL);
int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client = 0);
int DDraw_Flip(void);
int DDraw_Wait_For_Vsync(void);
LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list);
int DDraw_Init(int width, int height, int bpp, int windowed);
int DDraw_Shutdown(void);
int Load_Palette_From_File(char *filename, LPPALETTEENTRY palette);

#endif