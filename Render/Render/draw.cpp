#include "draw.h"



int DDraw_Fill_Surface(LPDIRECTDRAWSURFACE7 lpdds, USHORT color, RECT *client)
{
	DDBLTFX ddbltfx;

	DDRAW_INIT_STRUCT(ddbltfx);

	ddbltfx.dwFillColor = color;

	lpdds->Blt(client, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);

	return 1;
}

UCHAR *DDraw_Lock_Surface(LPDIRECTDRAWSURFACE7 lpdds, int *lpitch)
{
	if (!lpdds)
		return NULL;

	//DDRAW_INIT_STRUCT(ddsd)
}