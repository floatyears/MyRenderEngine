#include "poly.h"
#include "math3d.h"
#include "draw.h"
#include "camera.h"
#include "draw_windows.h"
#include "light.h"
#include "resource.h"
#include <windowsx.h>
#include <Windows.h>

#define WINDOW_CLASS_NAME	"WIN3DCLASS"
#define WINDOW_TITLE		"Sample 1"
#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		480

#define WINDOW_BPP			16
#define WINDOW_APP			1

#define AMBIENT_LIGHT_INDEX		0
#define INFINITE_LIGHT_INDEX	1

Camera4D cam;
RenderList4D render_list;
Poly4D poly;
Poly4D poly1;

RGBA white;
RGBA yellow;

Bitmap_File bitmap_file;
Bitmap_File bitmap_file1;

Point4D poly_pos = { 0, 0, 100, 1 };
Point4D  cam_pos = { 0, 0, -300, 1 };

//����ʹ�õ��ǻ���ֵ
Vector4D cam_dir = { 0, 0, 0, 1 };

ZBUFFER zbuffer;

int Game_Init(void *params = NULL)
{
	DDraw_Init(WINDOW_WIDTH,WINDOW_HEIGHT,WINDOW_BPP,WINDOW_APP);

	poly.state = POLY4D_STATE_ACTIVE;
	//poly.attr = POLY4D_ATTR_SHADE_MODE_CONSTANT;
	poly.attr = POLY4D_ATTR_SHADE_MODE_TEXTURE;
	//poly.lit_color[0] = 
	poly.vert_color[0] = RGBA32BIT(0, 255, 0, 255);
	poly.vert_color[1] = RGBA32BIT(255, 0, 0, 255);
	poly.vert_color[2] = RGBA32BIT(0, 0, 255, 255);

	poly.verts[0].x = -80;
	poly.verts[0].y = -140;
	poly.verts[0].z = 30;
	poly.verts[0].w = 1;
	poly.verts[0].u0 = 0.3;
	poly.verts[0].v0 = 0.5;


	poly.verts[1].x = -160;
	poly.verts[1].y = 0;
	poly.verts[1].z = 10;
	poly.verts[1].w = 1;
	poly.verts[1].u0 = 0;
	poly.verts[1].v0 = 0.7;

	poly.verts[2].x = 130;
	poly.verts[2].y = 52;
	poly.verts[2].z = 80;
	poly.verts[2].w = 1;
	poly.verts[2].u0 = 0.9;
	poly.verts[2].v0 = 0.9;

	poly.next = poly.prev = NULL;

	poly1.state = POLY4D_STATE_ACTIVE;
	//poly1.attr = POLY4D_ATTR_SHADE_MODE_CONSTANT;
	poly1.attr = POLY4D_ATTR_SHADE_MODE_TEXTURE;// POLY4D_ATTR_SHADE_MODE_GOURAUD;
	//poly1.lit_color[0] = 
	poly1.vert_color[0] = RGBA32BIT(255, 255, 0, 255);
	poly1.vert_color[1] = RGBA32BIT(255, 0, 0, 255);
	poly1.vert_color[2] = RGBA32BIT(0, 0, 255, 255);


	poly1.verts[0].x = -80;
	poly1.verts[0].y = -130;
	poly1.verts[0].z = 20;
	poly1.verts[0].w = 1;
	poly1.verts[0].u0 = 0.5;
	poly1.verts[0].v0 = 0;

	poly1.verts[1].x = 150;
	poly1.verts[1].y = 0;
	poly1.verts[1].z = 40;
	poly1.verts[1].w = 1;
	poly1.verts[1].u0 = 0.0;
	poly1.verts[1].v0 = 0.9;

	poly1.verts[2].x = 130;
	poly1.verts[2].y = 152;
	poly1.verts[2].z = 10;
	poly1.verts[2].w = 1;
	poly1.verts[2].u0 = 0.9;
	poly1.verts[2].v0 = 0.9;

	poly1.next = poly1.prev = NULL;

	Load_Bitmap_File(&bitmap_file, "./resource/cloud03.bmp");
	poly1.texture = (Bitmap_Iamge_PTR)malloc(sizeof(Bitmap_Image));
	Create_Bitmap(poly1.texture, 0, 0, bitmap_file.bitmapinfoheader.biWidth, bitmap_file.bitmapinfoheader.biHeight, bitmap_file.bitmapinfoheader.biBitCount);
	Load_Image_Bitmap(poly1.texture, &bitmap_file, 0, 0, BITMAP_EXTRACT_MODE_ABS);
	Unload_Bitmap_File(&bitmap_file);

	Load_Bitmap_File(&bitmap_file1, "./resource/cloud03.bmp");
	poly.texture = (Bitmap_Iamge_PTR)malloc(sizeof(Bitmap_Image));
	Create_Bitmap(poly.texture, 0, 0, bitmap_file1.bitmapinfoheader.biWidth, bitmap_file1.bitmapinfoheader.biHeight, bitmap_file1.bitmapinfoheader.biBitCount);
	Load_Image_Bitmap(poly.texture, &bitmap_file1, 0, 0, BITMAP_EXTRACT_MODE_ABS);
	Unload_Bitmap_File(&bitmap_file1);

	Init_Camera4D(&cam, CAM_MODEL_EULER, &cam_pos, &cam_dir,NULL, 50.0f, 500.0f, 120.0, WINDOW_WIDTH, WINDOW_HEIGHT);

	white.rgba = _RGBA32BIT(255,0,255,0);
	yellow.rgba = _RGBA32BIT(100, 100, 0, 0);

	Init_Lights_Light(AMBIENT_LIGHT_INDEX, LIGHT_STATE_ON, LIGHT_ATTR_AMBIENT, white, white, white, NULL, NULL, 0, 0, 0, 0, 0, 0);
	
	Vector4D dlight_dir = { -1, 0, -1, 0 };
	Init_Lights_Light(INFINITE_LIGHT_INDEX, LIGHT_STATE_ON, LIGHT_ATTR_INFINITE, white, yellow, white, NULL, &dlight_dir, 0, 0, 0, 0, 0, 0);

	Create_ZBuffer(&zbuffer, WINDOW_WIDTH, WINDOW_HEIGHT, ZBUFFER_ATTR_32BIT);
	return 1;
}

int Game_Main(void *params = NULL)
{
	static Matrix4X4 mrot;		//ͨ�õ���ת����
	static float ang_y = 0;		//��ת�Ƕ�

	//int index;					//ѭ������
	
	Start_Clock();

	//������Ƶ���
	DDraw_Fill_Surface(lpddsback, 0);

	// read keyboard and other devices here
	//DInput_Read_Keyboard();

	//��ʼ����Ⱦ�б�
	Reset_RenderList4D(&render_list);


	Insert_Poly4D_RenderList4D(&render_list, &poly);

	Insert_Poly4D_RenderList4D(&render_list, &poly1);


	Build_XYZ_Rotation_Matrix4X4(ang_y, 0, 0, &mrot);

	if ((ang_y += 0.01) >= 360.0) ang_y = 0;
	//if ((poly1.verts[0].u0 += 0.005) > 1.0) poly1.verts[0].u0 = 0.0;

	//��ת��������
	Transform_RenderList4D(&render_list, &mrot, TRANSFORM_LOCAL_ONLY);

	//ģ��λ�ñ仯����������
	Model_To_World_RenderList4D(&render_list, &poly_pos);

	//�����޳�
	//Remove_Backfaces_RenderList4D(&render_list, &cam);

	//����
	Light_RenderList4D_World(&render_list, &cam, lights, 2);

	//�������絽����ľ���
	Build_Camera4D_Matrix_Euler(&cam, CAM_ROT_SEQ_ZYX);
	
	//�������굽�������ı任
	Transform_RenderList4D(&render_list, &cam.mcam, TRANSFORM_TRANS_ONLY);

	//������ռ���вü�
	Clip_Poly_RenderList4D(&render_list, &cam, CLIP_POLY_Z_PLANE);

	//�����͸��
	Camera_To_Perspective_RenderList4D(&render_list, &cam);

	//͸�ӵ���Ļ
	Perspective_To_Screen_RenderList4D(&render_list, &cam);

	DDraw_Lock_Back_Surface();

	//�ȶ�zbuffer��������
	Clear_ZBuffer(&zbuffer, 0);
	//�����߿�
	Draw_RenderList4D_Wire(&render_list, back_buffer, back_lpitch);
	Draw_RenderList4D_Solid(&render_list, back_buffer, back_lpitch, zbuffer.zbuffer, WINDOW_WIDTH * 4);

	//int width = poly1.texture->width > max_clip_x ? max_clip_y : poly1.texture->width;
	//UINT *dest_buffer = (UINT *)back_buffer;
	//for (int yi = 0; yi < poly1.texture->height; yi++)
	//{
	//	if (yi > max_clip_y) continue;
	//	dest_buffer += yi * 640;
	//	for (int xi = 0; xi <= poly1.texture->width; xi++)
	//	{
	//		if (xi > max_clip_x) continue;
	//		//((UINT *)back_buffer)[xi] = ((UINT *)&poly1.texture->buffer)[yi*width + xi];
	//	}
	//}
	

	DDraw_Unlock_Back_Surface();

	DDraw_Flip();

	Wait_Clock(30);

	return 1;
}

void Game_ShutDown(void *params = NULL)
{
	DDraw_Shutdown();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT32 msg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (msg)
	{
	case WM_CREATE:
		return 0;
		break;
	case WM_PAINT:
		//��ʼ����
		hdc = BeginPaint(hwnd, &ps);

		//��������
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hprevinstance, LPSTR lpcmdline, int ncmdshow)
{
	WNDCLASS winclass;
	HWND hwnd;
	MSG msg;
	//HDC hdc;
	//PAINTSTRUCT ps;

	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;

	//ע��window��
	if (!RegisterClass(&winclass))
		return 0;

	//�������ھ��
	if (!(hwnd = CreateWindow(WINDOW_CLASS_NAME, WINDOW_TITLE, (WINDOW_APP ? (WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION) : (WS_POPUP | WS_VISIBLE)),
		0, 0,
		WINDOW_WIDTH, WINDOW_HEIGHT, NULL, NULL, hinstance, NULL)))
		return 0;

	main_window_handle = hwnd;
	main_instance = hinstance;

	if (WINDOW_APP)
	{
		RECT window_rect = { 0, 0, WINDOW_WIDTH - 1, WINDOW_HEIGHT - 1 };

		AdjustWindowRectEx(&window_rect, GetWindowStyle(main_window_handle), GetMenu(main_window_handle) != NULL, GetWindowExStyle(main_window_handle));

		window_client_x0 = -window_rect.left;
		window_client_y0 = -window_rect.top;

		MoveWindow(main_window_handle, 0, 0,
			window_rect.right - window_rect.left,
			window_rect.bottom - window_rect.top,
			FALSE);

		ShowWindow(main_window_handle, SW_SHOW);
	}

	Game_Init();

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);

			DispatchMessage(&msg);
		}

		Game_Main();
	}

	Game_ShutDown();

	return 1;
}

