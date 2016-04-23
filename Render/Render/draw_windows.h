#ifndef H_DRAWWINDOWS
#define H_DRAWWINDOWS

#include <Windows.h>
#include <ddraw.h>

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

extern HWND main_window_handle;
extern HINSTANCE main_instance;

extern LPDIRECTDRAWSURFACE7 lpddsprimary;
extern LPDIRECTDRAWSURFACE7 lpddsback;
extern LPDIRECTDRAWPALETTE  lpddpal;
extern LPDIRECTDRAWCLIPPER  lpddclipper;
extern LPDIRECTDRAWCLIPPER  lpddcliperwin;

extern DDSCAPS2			 ddscaps;
extern HRESULT				 ddrval;

extern PALETTEENTRY		 palette[MAX_COLORS_PALETTE];
extern PALETTEENTRY		 save_palette[MAX_COLORS_PALETTE];

extern DDSURFACEDESC2 ddsd;
extern LPDIRECTDRAW7 lpdd ;
extern UCHAR *primary_buffer ;
extern UCHAR *back_buffer ;

extern int dd_pixel_format;
extern int primary_lpitch ;
extern int back_lpitch ;


extern int min_clip_x,
    max_clip_x ,
    min_clip_y ,
    max_clip_y;

extern int screen_width ,
    screen_height ,
    screen_bpp ,
    screen_windowed;

extern int window_client_x0;
extern int window_client_y0;

//RGB16µÄº¯ÊýÖ¸Õë
extern USHORT (*RGB16Bit)(int r, int g, int b);
extern DWORD start_clock_count;

#define _RGBA32BIT(r,g,b,a) ((a) + ((b) << 8) + ((g) << 16) + ((r) << 24))
#define _RGB555FROM16BIT(RGB,r,g,b) { *r = (((RGB) >> 10) & 0x1f); *g = (((RGB) >> 5) & 0x1f); *b = ((RGB) & 0x1f); }
#define _RGB565FROM16BIT(RGB,r,g,b) { *r = (((RGB) >> 11) & 0x1f); *g = (((RGB) >> 5) & 0x3f); *b = ((RGB) & 0x1f); }
#define _RGBFROM32BIT(rbga,r,g,b) { *r = (((rbga) >> 16) & 0xff); *g = (((rbga) >> 8) & 0xff); *b = ((rbga) & 0xff);}
#define _RGBAFROM32BIT(rbga,r,g,b,a) { *a = (((rbga) >> 24) & 0xff); *r = (((rbga) >> 16) & 0xff); *g = (((rbga) >> 8) & 0xff); *b = ((RGB) & 0xff);}

#define _RGB16BIT555(r,g,b) ((b & 31) + ((g & 31) << 5) + ((r & 31) << 10))
#define _RGB16BIT565(r,g,b) ((b & 31) + ((g & 63) << 5) + ((r & 31) << 11))
#define _RGB24BIT(r,g,b)	(b + g << 8 + r << 16)
#define _RGB32BIT(a,r,g,b)	((b) + ((g) << 8) + ((r) << 16) + ((a) << 24))

USHORT RGB16Bit565(int r, int g, int b);
USHORT RGB16Bit555(int r, int g, int b);
int RGBA32BIT(int r, int g, int b, int a);
UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds, int *lpitch);
int DDraw_Unlock_Surface(LPDIRECTDRAWSURFACE7 lpdds);
UCHAR *DDraw_Lock_Primary_Surface(void);
int DDraw_Unlock_Primary_Surface(void);
UCHAR *DDraw_Lock_Back_Surface(void);
int DDraw_Unlock_Back_Surface(void);
LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, long mem_flags = 0, USHORT color_key_value = 0);
int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client = 0);
int DDraw_Flip(void);
int DDraw_Wait_For_Vsync(void);
LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list);
int DDraw_Init(int width, int height, int bpp, int windowed);
int DDraw_Shutdown(void);
int Load_Palette_From_File(char *filename, LPPALETTEENTRY palette);

DWORD Start_Clock(void);
DWORD Get_Clock(void);
DWORD Wait_Clock(DWORD count);

#endif
