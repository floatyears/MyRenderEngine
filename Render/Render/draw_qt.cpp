#include "draw_qt.h"
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <malloc/malloc.h>
//#include <wingdi.h>
//#include "fbx_loader/ai_defs.h"

UCHAR *primary_buffer = 0;
UCHAR *back_buffer = 0;

int dd_pixel_format = DD_PIXEL_FORMAT565;
int primary_lpitch = 0;
int back_lpitch = 0;


int min_clip_x = 0,
max_clip_x = WINDOW_WIDTH - 1,
min_clip_y = 0,
max_clip_y = WINDOW_HEIGHT - 1;

//int screen_width = SCREEN_WIDTH,
//screen_height = SCREEN_HEIGHT,
//screen_bpp = SCREEN_BPP,
//screen_windowed = 0;

int window_client_x0 = 0;
int window_client_y0 = 0;

DWORD start_clock_count = 0;

USHORT(*RGB16Bit)(int r, int g, int b) = 0;

USHORT RGB16Bit565(int r, int g, int b)
{
    r >>= 3; g >>= 2; b >>= 3;
    return _RGB16BIT565(r, g, b);
}

USHORT RGB16Bit555(int r, int g, int b)
{
    r >>= 3;
    g >>= 3;
    b >>= 3;
    return _RGB16BIT555(r, g, b);
}

int RGBA32BIT(int r, int g, int b, int a)
{
    return _RGB32BIT(a, r, g, b);
}

void Clear_Back_Buffer(UCHAR* back_buffer, UCHAR value)
{
    for(int i = 0; i < WINDOW_HEIGHT * WINDOW_WIDTH * 4;i++){
        back_buffer++;
        *back_buffer = value;
    }
}

UCHAR *DDraw_Lock_Back_Surface(void)
{
//    if (back_buffer)
//        return back_buffer;

//    DDRAW_INIT_STRUCT(ddsd);
//    lpddsback->Lock(NULL,&ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);

//    back_buffer = (UCHAR *)ddsd.lpSurface;
//    //back_lpitch = 640;//
//    back_lpitch = ddsd.lPitch;

    return back_buffer;
}

int DDraw_Init(int width, int height/*, int bpp, int windowed*/)
{
    back_buffer = (UCHAR* )malloc(sizeof(UCHAR) * WINDOW_WIDTH * WINDOW_HEIGHT*4);
    back_lpitch = WINDOW_WIDTH * 4;
    return 0;
}



DWORD Start_Clock(void)
{
    return start_clock_count = Get_Clock();
}

DWORD Get_Clock(void)
{
    return GetTickCount();

}

DWORD GetTickCount()
{
    return 0;
}


DWORD Wait_Clock(DWORD count)
{
    //while (Get_Clock() - start_clock_count < count);
    return Get_Clock();
}
