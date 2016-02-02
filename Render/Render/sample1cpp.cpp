#include "draw.h"
#include <windowsx.h>
#include <windef.h>
#include "camera.h"
#include "poly.h"

#define WINDOW_CLASS_NAME	"WIN3DCLASS"
#define WINDOW_TITLE		"Sample 1"
#define WINDOW_WIDTH		640
#define WINDOW_HEIGHT		480

#define WINDOW_BPP			16
#define WINDOW_APP			1

Camera4D cam;
RenderList4D render_list;
Poly4D poly;
Point4D poly_pos = { 0, 0, 100, 1 };
Point4D  cam_pos = { 0, 0, 0, 1 };
Vector4D cam_dir = { 0, 0, 0, 1 };




int Game_Init(void *params)
{
	DDraw_Init(WINDOW_WIDTH,WINDOW_HEIGHT,WINDOW_BPP,WINDOW_APP);

	poly.state = POLY4D_STATE_ACTIVE;
	poly.attr = 0;
	poly.color = RGB16Bit(0, 255, 0);

	poly.verts[0].x = 0;
	poly.verts[0].y = 50;
	poly.verts[0].z = 0;
	poly.verts[0].w = 1;

	poly.verts[1].x = 50;
	poly.verts[1].y = 50;
	poly.verts[1].z = 0;
	poly.verts[1].w = 1;

	poly.verts[2].x = -50;
	poly.verts[2].y = -50;
	poly.verts[2].z = 0;
	poly.verts[2].w = 1;

	poly.next = poly.prev = NULL;
	Init_Camera4D(&cam, CAM_MODEL_EULER, &cam_pos, &cam_dir,NULL, 50.0, 500.0, 90.0, WINDOW_WIDTH, WINDOW_HEIGHT);
	return 1;
}

int Game_Main(void *params)
{
	static Matrix4X4 mrot;		//通用的旋转矩阵
	static float ang_y = 0;		//旋转角度

	int index;					//循环变量
	
	//清理绘制的面
	DDraw_Fill_Surface(lpddsback, 0);

	//初始化渲染列表
	Reset_RenderList4D(&render_list);

	Insert_Poly4D_RenderList4D(&render_list, &poly);

	Build_XYZ_Rotation_Matrix4X4(0, ang_y, 0, &mrot);

	if (++ang_y >= 360.0) ang_y = 0;

	//旋转本地坐标
	Transform_RenderList4D(&render_list, &mrot, TRANSFORM_LOCAL_ONLY);


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
		//开始绘制
		hdc = BeginPaint(hwnd, &ps);

		//结束绘制
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
	HDC hdc;
	PAINTSTRUCT ps;

	winclass.style = CS_DBLCLKS | CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;

	//注册window类
	if (!RegisterClass(&winclass))
		return 0;

	//创建窗口句柄
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
}