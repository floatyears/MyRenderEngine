#include "draw_windows.h"
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <wingdi.h>

HWND main_window_handle = 0;
HINSTANCE main_instance = 0;

LPDIRECTDRAWSURFACE7 lpddsprimary = 0;
LPDIRECTDRAWSURFACE7 lpddsback = 0;
LPDIRECTDRAWPALETTE  lpddpal = 0;
LPDIRECTDRAWCLIPPER  lpddclipper = 0;
LPDIRECTDRAWCLIPPER  lpddcliperwin = 0;

DDSCAPS2			 ddscaps;
HRESULT				 ddrval;

PALETTEENTRY		 palette[MAX_COLORS_PALETTE];
PALETTEENTRY		 save_palette[MAX_COLORS_PALETTE];

DDSURFACEDESC2 ddsd;
LPDIRECTDRAW7 lpdd = 0;
UCHAR *primary_buffer = 0;
UCHAR *back_buffer = 0;

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

int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client)
{
    DDBLTFX ddbltfx;

    DDRAW_INIT_STRUCT(ddbltfx);

    ddbltfx.dwFillColor = color;

    lpdds->Blt(client, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

    return 1;
}

//此函数锁定已发送的表面，并返回一个指向它的指针
UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds, int *lpitch)
{
    if (!lpdds)
        return NULL;

    DDRAW_INIT_STRUCT(ddsd);
    lpdds->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);

    //设置内存间距
    if (lpitch)
        *lpitch = ddsd.lPitch;

    return (UCHAR *)ddsd.lpSurface;

}

int DDraw_Unlock_Surface(LPDIRECTDRAWSURFACE7 lpdds)
{
    if (!lpdds)
    {
        return 0;
    }

    lpdds->Unlock(NULL);

    return 1;
}

UCHAR *DDraw_Lock_Primary_Surface(void)
{
    if (primary_buffer)
        return primary_buffer;

    DDRAW_INIT_STRUCT(ddsd);
    lpddsprimary->Lock(NULL, &ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);

    primary_buffer = (UCHAR *)ddsd.lpSurface;
    primary_lpitch = ddsd.lPitch;

    return primary_buffer;
}

int DDraw_Unlock_Primary_Surface(void)
{
    if (!primary_buffer)
        return 0;

    lpddsprimary->Unlock(NULL);

    primary_buffer = NULL;
    primary_lpitch = 0;

    return 1;
}

UCHAR *DDraw_Lock_Back_Surface(void)
{
    if (back_buffer)
        return back_buffer;

    DDRAW_INIT_STRUCT(ddsd);
    lpddsback->Lock(NULL,&ddsd, DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR, NULL);

    back_buffer = (UCHAR *)ddsd.lpSurface;
    //back_lpitch = 640;//
    back_lpitch = ddsd.lPitch;

    return back_buffer;
}

int DDraw_Unlock_Back_Surface(void)
{
    if (!back_buffer)
        return 0;

    lpddsback->Unlock(NULL);

    back_buffer = NULL;
    back_lpitch = 0;

    return 1;
}

LPDIRECTDRAWSURFACE7 DDraw_Create_Surface(int width, int height, long mem_flags, USHORT color_key_value)
{
    DDSURFACEDESC2 ddsd;
    LPDIRECTDRAWSURFACE7 lpdds;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;

    ddsd.dwWidth = width;
    ddsd.dwHeight = height;

    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | mem_flags;

    if (FAILED(lpdd->CreateSurface(&ddsd, &lpdds, NULL)))
    {
        return NULL;
    }

    //设置color key为默认的颜色000
    DDCOLORKEY color_key;
    color_key.dwColorSpaceLowValue = color_key_value;
    color_key.dwColorSpaceHighValue = color_key_value;

    //给source blitting设置color key
    lpdds->SetColorKey(DDCKEY_SRCBLT, &color_key);
    return lpdds;
}

int DDraw_Flip(void)
{
    if (primary_buffer || back_buffer)
        return 0;

    if (!screen_windowed)
        while ((FAILED(lpddsprimary->Flip(NULL, DDFLIP_WAIT))));
    else
    {
        RECT dest_rect;

        GetWindowRect(main_window_handle, &dest_rect);

        dest_rect.left += window_client_x0;
        dest_rect.top += window_client_y0;

        dest_rect.right = dest_rect.left + screen_width - 1;
        dest_rect.bottom = dest_rect.top + screen_height - 1;

        if (FAILED(lpddsprimary->Blt(&dest_rect, lpddsback, NULL, DDBLT_WAIT, NULL)))
            return 0;
    }

    return 1;
}

int DDraw_Wait_For_Vsync(void)
{
    lpdd->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
    return 1;
}

LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list)
{
    LPDIRECTDRAWCLIPPER lpddclipper;
    LPRGNDATA region_data;

    int index;
    if (FAILED(lpdd->CreateClipper(0, &lpddclipper, NULL)))
        return NULL;

    region_data = (LPRGNDATA)malloc(sizeof(RGNDATAHEADER) + num_rects*sizeof(RECT));

    memcpy(region_data->Buffer, clip_list, sizeof(RECT)*num_rects);

    region_data->rdh.dwSize = sizeof(RGNDATAHEADER);
    region_data->rdh.iType = RDH_RECTANGLES;
    region_data->rdh.nCount = num_rects;
    region_data->rdh.nRgnSize = num_rects*sizeof(RECT);

    region_data->rdh.rcBound.left = 64000;
    region_data->rdh.rcBound.top = 64000;
    region_data->rdh.rcBound.right = -64000;
    region_data->rdh.rcBound.bottom = -64000;

    // find bounds of all clipping regions
    for (index = 0; index<num_rects; index++)
    {
        // test if the next rectangle unioned with the current bound is larger
        if (clip_list[index].left < region_data->rdh.rcBound.left)
            region_data->rdh.rcBound.left = clip_list[index].left;

        if (clip_list[index].right > region_data->rdh.rcBound.right)
            region_data->rdh.rcBound.right = clip_list[index].right;

        if (clip_list[index].top < region_data->rdh.rcBound.top)
            region_data->rdh.rcBound.top = clip_list[index].top;

        if (clip_list[index].bottom > region_data->rdh.rcBound.bottom)
            region_data->rdh.rcBound.bottom = clip_list[index].bottom;

    } // end for index

    // now we have computed the bounding rectangle region and set up the data
    // now let's set the clipping list

    if (FAILED(lpddclipper->SetClipList(region_data, 0)))
    {
        // release memory and return error
        free(region_data);
        return(NULL);
    } // end if

    // now attach the clipper to the surface
    if (FAILED(lpdds->SetClipper(lpddclipper)))
    {
        // release memory and return error
        free(region_data);
        return(NULL);
    } // end if

    // all is well, so release memory and send back the pointer to the new clipper
    free(region_data);
    return(lpddclipper);
}

int DDraw_Init(int width, int height, int bpp, int windowed)
{
    //int index;
    HRESULT hr = DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL);
    if (FAILED(hr))
        return 0;
    if (windowed)
    {
        if (FAILED(lpdd->SetCooperativeLevel(main_window_handle, DDSCL_NORMAL)))
            return 0;
    }
    else
    {
        if (FAILED(lpdd->SetCooperativeLevel(main_window_handle, DDSCL_ALLOWMODEX | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX | DDSCL_MULTITHREADED)))
            return 0;
        if (FAILED(lpdd->SetDisplayMode(windowed, height, bpp, 0, 0)))
            return 0;
    }
    screen_height = height;
    screen_width = width;
    screen_bpp = bpp;
    screen_windowed = windowed;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);

    if (!screen_windowed)
    {
        ddsd.dwSize = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;

        ddsd.dwBackBufferCount = 1;
    }
    else
    {
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        ddsd.dwBackBufferCount = 0;
    }

    lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL);

    DDPIXELFORMAT ddpf;

    DDRAW_INIT_STRUCT(ddpf);
    lpddsprimary->GetPixelFormat(&ddpf);

    dd_pixel_format = ddpf.dwRGBBitCount;

    if(dd_pixel_format == DD_PIXEL_FORMAT555)
    {
        RGB16Bit = RGB16Bit555;
    }
    else
    {
        RGB16Bit = RGB16Bit565;
    }

    if (!screen_windowed)
    {
        ddscaps.dwCaps = DDSCAPS_BACKBUFFER;

        if (FAILED(lpddsprimary->GetAttachedSurface(&ddscaps, &lpddsback)))
            return 0;
    }
    else
    {
        lpddsback = DDraw_Create_Surface(width, height, DDSCAPS_SYSTEMMEMORY,NULL);
    }

    if (screen_bpp == DD_PIXEL_FORMAT8)
    {
        memset(palette, 0, MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));
        Load_Palette_From_File("PALDATA2.PAL",palette);

        if (screen_windowed)
        {
            for (int index = 0; index < 10; index++)
            {
                palette[index].peFlags = palette[index + 246].peFlags = PC_EXPLICIT;

                if (FAILED(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE, palette, &lpddpal, NULL)))
                    return 0;
            }
        }
        else
        {
            if (FAILED(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256, palette, &lpddpal, NULL)))
                return 0;
        }

        if (FAILED(lpddsprimary->SetPalette(lpddpal)))
            return 0;
    }

    if (screen_windowed)
    {
        DDraw_Fill_Surface(lpddsback, 0);
    }
    else
    {
        DDraw_Fill_Surface(lpddsprimary, 0);
        DDraw_Fill_Surface(lpddsprimary, 0);
    }

    min_clip_x = 0;
    max_clip_x = screen_width - 1;
    min_clip_y = 0;
    max_clip_y = screen_height - 1;

    RECT screen_rect = { 0, 0, screen_width, screen_height };
    lpddclipper = DDraw_Attach_Clipper(lpddsback,1,&screen_rect);

    if (screen_windowed)
    {
        if (FAILED(lpdd->CreateClipper(0, &lpddcliperwin, NULL)))
            return 0;
        if (FAILED(lpddcliperwin->SetHWnd(0, main_window_handle)))
            return 0;
        if (FAILED(lpddsprimary->SetClipper(lpddcliperwin)))
            return 0;
    }
    return 1;
}

int DDraw_Shutdown(void)
{
    if (lpddclipper)
        lpddclipper->Release();

    if (lpddcliperwin)
        lpddcliperwin->Release();

    if (lpddsback)
        lpddsback->Release();

    if (lpddsprimary)
        lpddsprimary->Release();

    if (lpdd)
        lpdd->Release();

    return 1;
}

int Load_Palette_From_File(char *filename, LPPALETTEENTRY palette)
{
    return 0;

    /*FILE *fp_file;
    if ((fp_file = fopen(filename, "r")) == NULL)
        return 0;
    for (int index = 0; index < MAX_COLORS_PALETTE; index++)
    {
        fscanf(fp_file, "%d %d %d %d", &palette[index].peRed, &palette[index].peGreen, &palette[index].peBlue, &palette[index].peFlags);

        fclose(fp_file);
    }

    return 1;*/

}

DWORD Start_Clock(void)
{
    return start_clock_count = Get_Clock();
}

DWORD Get_Clock(void)
{
    return GetTickCount();
}

DWORD Wait_Clock(DWORD count)
{
    while (Get_Clock() - start_clock_count < count);
    return Get_Clock();
}
