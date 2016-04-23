#include "draw.h"
//#include <minwindef.h>
#include "poly.h"
#include "draw_qt.h"
#include <math.h>
#include <stdlib.h>

int Draw_Line(int x0, int y0, int x1, int y1, int color, UCHAR *_vb_start, int lpitch)
{
    int dx,
        dy,
        dx2,
        dy2,
        x_inc, //绘制时在像素空间要移动的像素数量
        y_inc, //绘制时在像素空间要移动的像素数量
        error,
        index;

    //转换为16进制
    UINT *vb_start = (UINT *)_vb_start;
    //重新计算32位下的pitch
    lpitch = lpitch >> 2;

    //预计算视频缓冲中的第一个像素地址
    vb_start = vb_start + x0 + y0*lpitch;

    dx = x1 - x0;
    dy = y1 - y0;

    //检测直线指向的方向，例如斜率角度
    if (dx >= 0)
    {
        x_inc = 1;
    }
    else
    {
        x_inc = -1;
        dx = -dx;
    }

    //检测y的斜率
    if (dy >= 0)
    {
        y_inc = lpitch;
    }
    else
    {
        y_inc = -lpitch;
        dy = -dy;
    }

    //计算 (dx,dy)*2
    dx2 = dx << 1;
    dy2 = dy << 1;

    //根据哪个delta更大，我们可以绘制这条直线
    if (dx > dy)
    {
        //初始化错误参数
        error = dy2 - dx;

        for (index = 0; index <= dx; index++)
        {
            *vb_start = color;

            //检测错误码是否溢出
            if (error >= 0)
            {
                error -= dx2;

                //移动到下一行
                vb_start += y_inc;
            }

            //调整错误参数
            error += dy2;

            //移动到下一个像素
            vb_start += x_inc;
        }
    }
    else
    {
        error = dx2 - dy;

        for ( index = 0; index <= dy; index++)
        {
            *vb_start = color;

            if (error >= 0)
            {
                error -= dy2;
                vb_start += x_inc;
            }

            error += dx2;
            vb_start += y_inc;
        }
    }

    return 1;
}

int Draw_Pixel(int x, int y, int color, UCHAR *video_buffer, int lpitch)
{
    video_buffer[x + y*lpitch] = color;
    return 1;
}

int Clip_Line(int &x1, int &y1, int &x2, int &y2)
{
#define CLIP_CODE_C		0x0000
#define CLIP_CODE_N		0x0008
#define CLIP_CODE_S		0x0004
#define CLIP_CODE_E		0x0002
#define CLIP_CODE_W		0x0001

#define CLIP_CODE_NE	0x000a
#define CLIP_CODE_SE	0x0006
#define CLIP_CODE_NW	0x0009
#define CLIP_CODE_SW	0x0005

    int xc1 = x1,
        yc1 = y1,
        xc2 = x2,
        yc2 = y2;

    int p1_code = 0,
        p2_code = 0;

    if (y1 < min_clip_y)
        p1_code |= CLIP_CODE_N;
    else if (y1 > max_clip_y)
        p1_code |= CLIP_CODE_S;

    if (x1 < min_clip_x)
        p1_code |= CLIP_CODE_W;
    else if (x1 > max_clip_x)
        p1_code |= CLIP_CODE_E;

    if (y2 < min_clip_y)
        p2_code |= CLIP_CODE_N;
    else if (y2 > max_clip_y)
        p2_code |= CLIP_CODE_S;

    if (x2 < min_clip_x)
        p2_code |= CLIP_CODE_W;
    else if (x2 > max_clip_x)
        p2_code |= CLIP_CODE_E;

    if (p1_code & p2_code)
        return 0;

    if (p1_code == 0 && p2_code == 0)
        return 1;

    switch (p1_code)
    {
    case CLIP_CODE_C:
        break;
    case CLIP_CODE_N:
        yc1 = min_clip_y;
        xc1 = x1 + 0.5f + (min_clip_y - y1)*(x2 - x1) / (y2 - y1);
        break;
    case CLIP_CODE_S:
        yc1 = max_clip_y;
        xc1 = x1 + 0.5f + (max_clip_y - y1)*(x2 - x1) / (y2 - y1);
        break;
    case CLIP_CODE_W:
        xc1 = min_clip_x;
        yc1 = y1 + 0.5f + (min_clip_x - x1)*(y2 - y1) / (x2 - x1);
        break;
    case CLIP_CODE_E:
        xc1 = max_clip_x;
        yc1 = y1 + 0.5f + (max_clip_x - x1)*(y2 - y1) / (x2 - x1);
        break;
    case CLIP_CODE_NE:
        yc1 = min_clip_y;
        xc1 = x1 + 0.5f + (min_clip_y - y1)*(x2 - x1) / (y2 - y1);

        if (xc1 < min_clip_x || xc1 > max_clip_x)
        {
            xc1 = max_clip_x;
            yc1 = y1 + 0.5f + (max_clip_x - x1)*(y2 - y1) / (y2 - y1);
        }
        break;
    case CLIP_CODE_SE:
        yc1 = max_clip_y;
        xc1 = x1 + 0.5f + (max_clip_y - y1)*(x2 - x1) / (y2 - y1);

        if (xc1 < min_clip_x || xc1 > max_clip_x)
        {
            xc1 = max_clip_x;
            yc1 = y1 + 0.5f + (max_clip_x - x1)*(y2 - y1) / (x2 - x1);
        }
        break;
    case CLIP_CODE_NW:
    {
        // north hline intersection
        yc1 = min_clip_y;
        xc1 = x1 + 0.5f + (min_clip_y - y1)*(x2 - x1) / (y2 - y1);

        // test if intersection is valid, of so then done, else compute next
        if (xc1 < min_clip_x || xc1 > max_clip_x)
        {
            xc1 = min_clip_x;
            yc1 = y1 + 0.5f + (min_clip_x - x1)*(y2 - y1) / (x2 - x1);
        } // end if

    }
    case CLIP_CODE_SW:
        yc1 = max_clip_y;
        xc1 = x1 + 0.5f + (max_clip_y - y1)*(x2 - x1) / (y2 - y1);

        if (xc1 < min_clip_x || xc1 > max_clip_x)
        {
            xc1 = min_clip_x;
            yc1 = y1 + 0.5f + (min_clip_x - x1) / (y2 - y1) / (x2 - x1);
        }
        break;
    default:
        break;
    }
    switch (p2_code)
    {
    case CLIP_CODE_C:
        break;
    case CLIP_CODE_N:
        yc2 = min_clip_y;
        xc2 = x2 + (min_clip_y - y2)*(x1 - x2) / (y1 - y2);
        break;
    case CLIP_CODE_S:
        yc2 = max_clip_y;
        xc2 = x2 + (max_clip_y - y2)*(x1 - x2) / (y1 - y2);
        break;
    case CLIP_CODE_W:
        xc2 = min_clip_x;
        yc2 = y2 + (min_clip_x - x2)*(y1 - y2) / (x1 - x2);
        break;
    case CLIP_CODE_E:
        xc2 = max_clip_x;
        yc2 = y2 + (max_clip_x - x2)*(y1 - y2) / (x1 - x2);
        break;
    case CLIP_CODE_NE:
        yc2 = min_clip_y;
        xc2 = x2 + 0.5 + (min_clip_y - y2)*(y1 - y2) / (x1 - x2);
        if (xc2 < min_clip_x || xc2 > max_clip_x)
        {
            // east vline intersection
            xc2 = max_clip_x;
            yc2 = y2 + 0.5 + (max_clip_x - x2)*(y1 - y2) / (x1 - x2);
        } // end if
        break;
    case CLIP_CODE_SE:
        yc2 = max_clip_y;
        xc2 = x2 + 0.5 + (max_clip_y - y2)*(x1 - x2) / (y1 - y2);
        if (xc2 < min_clip_x || xc2 > max_clip_x)
        {
            xc2 = max_clip_x;
            yc2 = y2 + 0.5 + (max_clip_x - x2)*(y1 - y2) / (x1 - x2);
        }
        break;
    case CLIP_CODE_NW:
        yc2 = min_clip_y;
        xc2 = x2 + 0.5 + (min_clip_y - y2)*(x1 - x2) / (y1 - y2);

        // test if intersection is valid, of so then done, else compute next
        if (xc2 < min_clip_x || xc2 > max_clip_x)
        {
            xc2 = min_clip_x;
            yc2 = y2 + 0.5 + (min_clip_x - x2)*(y1 - y2) / (x1 - x2);
        } // end if

        break;
    case CLIP_CODE_SW:
        // south hline intersection
        yc2 = max_clip_y;
        xc2 = x2 + 0.5 + (max_clip_y - y2)*(x1 - x2) / (y1 - y2);

        // test if intersection is valid, of so then done, else compute next
        if (xc2 < min_clip_x || xc2 > max_clip_x)
        {
            xc2 = min_clip_x;
            yc2 = y2 + 0.5 + (min_clip_x - x2)*(y1 - y2) / (x1 - x2);
        } // end if

    default:
        break;
    }

    //边界检查
    if (xc1 < min_clip_x || xc1 > max_clip_x || yc1 < min_clip_y || yc1 > max_clip_y || xc2 < min_clip_x || xc2 > max_clip_x || yc2 < min_clip_y || yc2 > max_clip_y)
        return 0;

    //存储回变量
    x1 = xc1;
    y1 = yc1;
    x2 = xc2;
    y2 = yc2;

    return 1;
}

int Draw_Clip_Line(int x0, int y0, int x1, int y1, int color, UCHAR *dest_buffer, int lpitch)
{
    int cxs, cys,
        cxe, cye;

    cxs = x0;
    cys = y0;
    cxe = x1;
    cye = y1;


    //裁剪直线
    if (Clip_Line(cxs, cys, cxe, cye))
        Draw_Line(cxs, cys, cxe, cye, color, dest_buffer, lpitch);

    return 1;
}

void Draw_RenderList4D_Wire(RenderList4D_PTR render_list, UCHAR *video_buffer, int lpitch)
{
    for (int poly = 0; poly < render_list->num_polys; poly++)
    {
        Poly4D_PTR curr_poly = render_list->poly_ptrs[poly];
        if (!(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->state & POLY4D_STATE_CLIPPED)
            continue;

        Draw_Clip_Line(curr_poly->tverts[0].x, curr_poly->tverts[0].y, curr_poly->tverts[1].x, curr_poly->tverts[1].y, curr_poly->vert_color[0], video_buffer, lpitch);
        Draw_Clip_Line(curr_poly->tverts[0].x, curr_poly->tverts[0].y, curr_poly->tverts[2].x, curr_poly->tverts[2].y, curr_poly->vert_color[0], video_buffer, lpitch);
        Draw_Clip_Line(curr_poly->tverts[1].x, curr_poly->tverts[1].y, curr_poly->tverts[2].x, curr_poly->tverts[2].y, curr_poly->vert_color[0], video_buffer, lpitch);

    }
}

void Draw_RenderList4D_Solid(RenderList4D_PTR render_list, UCHAR *video_buffer, int lpitch, UCHAR *zbuffer, int zpitch)
{
    for (int poly = 0; poly < render_list->num_polys; poly++)
    {
        Poly4D_PTR curr_poly = render_list->poly_ptrs[poly];
        if (!(curr_poly->state & POLY4D_STATE_ACTIVE) || curr_poly->state & POLY4D_STATE_BACKFACE || curr_poly->state & POLY4D_STATE_CLIPPED)
            continue;

        if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_CONSTANT)
        {
            Draw_Triangle_2D(curr_poly->tverts[0].x, curr_poly->tverts[0].y, curr_poly->tverts[1].x, curr_poly->tverts[1].y, curr_poly->tverts[2].x, curr_poly->tverts[2].y, curr_poly->lit_color[0], video_buffer, lpitch);
        }
        else if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_GOURAUD)
        {
            Draw_Gouraud_Triangle_Float(curr_poly, video_buffer, lpitch,zbuffer,zpitch);
        }
        else if (curr_poly->attr & POLY4D_ATTR_SHADE_MODE_TEXTURE)
        {
            Draw_Textured_Triangle(curr_poly, video_buffer, lpitch, zbuffer, zpitch);
        }
    }
}

void Draw_Top_Triangle(int x1, int y1, int x2, int y2, int x3, int y3, int color, UCHAR *_dest_buffer, int mempitch)
{
    //这个函数绘制一个三角形，它的顶部是平的
    //     x1,y1  —————————— x2,y2
    //            \        /
    //             \      /
    //              \    /
    //               \  /
    //                \/
    //               x3,y3
    float dx_right,
        dx_left,
        xs, xe,
        height;

    int tmp_x,
        tmp_y,
        right,
        left;

    //把目标buffer转换为ushort
    UINT *dest_buffer = (UINT *)_dest_buffer;

    //下一条扫描线的目标地址
    UINT *dest_addr = NULL;

    //以16位长度重新计算mempitch
    mempitch = (mempitch >> 2);

    if (x2 < x1)
    {
        tmp_x = x2;
        x2 = x1;
        x1 = tmp_x;
    }

    height = y3 - y1;

    dx_left = (x3 - x1) / height;
    dx_right = (x3 - x2) / height;

    xs = (float)x1;
    xe = (float)x2; //+(float)0.5

    if (y1 < min_clip_y)
    {
        xs = xs + dx_left * (float)(-y1 + min_clip_y);
        xe = xe + dx_right *(float)(-y1 + min_clip_y);

        y1 = min_clip_y;
    }

    if (y3 > max_clip_y)
    {
        y3 = max_clip_y;
    }


    dest_addr = dest_buffer + y1*mempitch;

    //检测是否需要裁剪x边
    if (x1 >= min_clip_x && x1 <= max_clip_x && x2 >= min_clip_x && x2 <= max_clip_x && x3 >= min_clip_x && x3 <= max_clip_x)
    {
        for (tmp_y = y1; tmp_y <= y3; tmp_y++, dest_addr += mempitch)
        {
            //Mem_Set_WORD(dest_addr + (unsigned int)(xs), color, (unsigned int)((int)xe - (int)xs + 1));
            for (int i = xs; i < xe; i++)
            {
                dest_addr[i] = color;
            }

            xs += dx_left;
            xe += dx_right;
        }
    }
    else
    {
        for (tmp_y = y1; tmp_y <= y3; tmp_y++, dest_addr += mempitch)
        {
            left = (int)xs;
            right = (int)xe;

            xs += dx_left;
            xe += dx_right;

            if (left < min_clip_x)
            {
                left = min_clip_x;
                if (right < min_clip_x)
                    continue;
            }

            if (right > max_clip_x)
            {
                right = max_clip_x;

                if (left > max_clip_x)
                    continue;
            }

            Mem_Set_WORD(dest_addr + (unsigned int)left, color, (unsigned int)(right - left + 1));
        }
    }
}

void Draw_Bottom_Triangle(int x1, int y1, int x2, int y2, int x3, int y3, int color, UCHAR *_dest_buffer, int mempitch)
{
    //这个函数绘制一个三角形，它的底部是平的
    //               x1,y1
    //                /\
    //               /  \
    //              /    \
    //             /      \
    //            /        \
    //     x2,y2  —————————— x3,y3

    float dx_left,
        dx_right,
        xs,
        xe,
        height;

    float tmp_x,
        tmp_y,
        right,
        left;

    UINT *dest_buffer = (UINT *)_dest_buffer;

    UINT *dest_addr = NULL;

    //以32位长度重新计算mempitch
    mempitch = (mempitch >> 2);

    if (x3 < x2)
    {
        tmp_x = x2;
        x2 = x3;
        x3 = tmp_x;
    }

    height = y3 - y1;

    dx_left = (x2 - x1) / height;
    dx_right = (x3 - x1) / height;

    xs = (float)x1;
    xe = (float)x1;

    if (y1 < min_clip_y)
    {
        xs = xs + dx_left*(float)(-y1 + min_clip_y);
        xe = xe + dx_right * (float)(-y1 + min_clip_y);

        y1 = min_clip_y;
    }

    if (y3 > max_clip_y)
    {
        y3 = max_clip_y;
    }

    dest_addr = dest_buffer + y1 * mempitch;

    //检测x是否需要裁剪
    if (x1 >= min_clip_x && x1 <= max_clip_x && x2 >= min_clip_x && x2 <= max_clip_x && x3 >= min_clip_x && x3 <= max_clip_x)
    {
        for (tmp_y = y1; tmp_y < y3; tmp_y++, dest_addr += mempitch)
        {
            Mem_Set_WORD(dest_addr + (unsigned int)xs, color, (unsigned int)((int)xe - (int)xs + 1));

            xs += dx_left;
            xe += dx_right;
        }
    }
    else
    {
        for (tmp_y = y1; tmp_y < y3; tmp_y++, dest_addr += mempitch)
        {
            left = (int)xs;
            right = (int)xe;

            xs += dx_left;
            xe += dx_right;

            if (left < min_clip_x)
            {
                left = min_clip_x;

                if (right < min_clip_x)
                    continue;
            }

            if (right > max_clip_x)
            {
                right = max_clip_x;
                if (left > max_clip_x)
                    continue;
            }

            Mem_Set_WORD(dest_addr + (unsigned int)left, color, (unsigned int)(right - left));
        }
    }
}

void Draw_Triangle_2D(int x1, int y1, int x2, int y2, int x3, int y3, int color, UCHAR * dest_buffer, int mempitch)
{
    int tmp_x, tmp_y, new_x;

    if ((x1 == x2 && x2 == x3) || (y1 == y2 && y2 == y3))
        return;

    //把y的值从小到大排列，分别表示为y1,y2,y3
    if (y2 < y1)
    {
        tmp_x = x2;
        tmp_y = y2;
        x2 = x1;
        y2 = y1;
        x1 = tmp_x;
        y1 = tmp_y;
    }

    if (y3 < y1)
    {
        tmp_x = x3;
        tmp_y = y3;
        y3 = y1;
        x3 = x1;
        y1 = tmp_y;
        x1 = tmp_x;
    }

    if (y3 < y2)
    {
        tmp_x = x3;
        tmp_y = y3;
        x3 = x2;
        y3 = y2;
        x2 = tmp_x;
        y2 = tmp_y;
    }

    if (y3 < min_clip_y || y1 > max_clip_y || (x1 < min_clip_x && x2 < min_clip_x && x3 < min_clip_x) || (x1 >max_clip_x && x2 > max_clip_x && x3 > max_clip_x))
        return;

    if (y1 == y2)
    {
        Draw_Top_Triangle(x1, y1, x2, y2, x3, y3, color, dest_buffer, mempitch);
    }
    else if (y2 == y3)
    {
        Draw_Bottom_Triangle(x1, y1, x2, y2, x3, y3, color, dest_buffer, mempitch);
    }
    else
    {
        new_x = x1 + (int)(0.5 + (float)(y2 - y1)*(float)(x3 - x1) / (float)(y3 - y1));

        Draw_Bottom_Triangle(x1, y1, new_x, y2, x2, y2, color, dest_buffer, mempitch);
        Draw_Top_Triangle(x2, y2, new_x, y2, x3, y3, color, dest_buffer, mempitch);
    }

}

//绘制Gouraud模式的三角形
void Draw_Gouraud_Triangle(Poly4D_PTR poly, UCHAR *_dest_buffer, int mempitch)
{
    int v0 = 0, v1 = 1, v2 = 2;
    int tmp;
    int tri_type;

    //提取的每个顶点的基础颜色
    int r0_base, g0_base, b0_base,
        r1_base, g1_base, b1_base,
        r2_base, g2_base, b2_base;

    //y轴开始和结束的位置
    int yrestart, ystart, yend, xstart, xend;

    int x0, y0, x1, y1, x2, y2,
        tv0, tu0, tw0, tv1, tu1, tw1, tv2, tu2, tw2;
    int dy;
    int dxdyl, dudyl, dvdyl, dwdyl, dxdyr, dudyr, dvdyr, dwdyr;
    int xl, vl, ul, wl, xr, vr, ur, wr;
    int dx, du, dv, dw;
    int ui, vi, wi;
    int xi, yi;

    int dyl, dyr;
    int irestart = INTERP_LHS;

    UINT *screen_ptr = NULL,
        //* = NULL,
        //*textmap = NULL,
        *dest_buffer = (UINT *)_dest_buffer;

    //以16位长度重新计算mempitch
    mempitch = (mempitch >> 2);

    //首先进行裁剪测试
    if ((poly->tverts[v0].y < min_clip_y && poly->tverts[v1].y < min_clip_y && poly->tverts[v2].y < min_clip_y) ||
        (poly->tverts[v0].y > max_clip_y && poly->tverts[v1].y > max_clip_y && poly->tverts[v2].y > max_clip_y) ||
        (poly->tverts[v0].x < min_clip_x && poly->tverts[v1].x < min_clip_x && poly->tverts[v2].x < min_clip_x) ||
        (poly->tverts[v0].x > max_clip_x && poly->tverts[v1].x > max_clip_x && poly->tverts[v2].x > max_clip_x))
        return;

    //检测三角形是否退化为直线
    if ((poly->tverts[v0].x == poly->tverts[v1].x && poly->tverts[v1].x == poly->tverts[v2].x) || (poly->tverts[v0].y == poly->tverts[v1].y && poly->tverts[v1].y == poly->tverts[v2].y))
        return;

    //对顶点进行排序
    if (poly->tverts[v1].y < poly->tverts[v0].y)
        SWAP(v0, v1, tmp);
    if (poly->tverts[v2].y < poly->tverts[v0].y)
        SWAP(v2, v0, tmp);
    if (poly->tverts[v2].y < poly->tverts[v1].y)
        SWAP(v2, v1, tmp);

    if ((int)(poly->tverts[v0].y + 0.5) == (int)(poly->tverts[v1].y + 0.5))
    {
        tri_type = TRI_TYPE_FLAT_TOP;
        if (poly->tverts[v0].x > poly->tverts[v1].x)
            SWAP(v0, v1, tmp);
    }
    else if ((int)(poly->tverts[v1].y + 0.5) == (int)(poly->tverts[v2].y + 0.5))
    {
        tri_type = TRI_TYPE_FLAT_BOTTOM;
        if (poly->tverts[v1].x > poly->tverts[v2].x)
            SWAP(v1, v2, tmp);
    }
    else
        tri_type = TRI_TYPE_GENERAL;

    _RGB555FROM16BIT(poly->lit_color[v0], &r0_base, &g0_base, &b0_base);
    _RGB555FROM16BIT(poly->lit_color[v1], &r1_base, &g1_base, &b1_base);
    _RGB555FROM16BIT(poly->lit_color[v2], &r2_base, &g2_base, &b2_base);

    r0_base <<= 3;
    g0_base <<= 2;
    b0_base <<= 3;

    r1_base <<= 3;
    g1_base <<= 2;
    b1_base <<= 3;

    r2_base <<= 3;
    g2_base <<= 2;
    b2_base <<= 3;

    //
    x0 = (int)(poly->tverts[v0].x + 0.5);
    y0 = (int)(poly->tverts[v0].y + 0.5);

    tu0 = r0_base;
    tv0 = g0_base;
    tw0 = b0_base;

    x1 = (int)(poly->tverts[v1].x + 0.5);
    y1 = (int)(poly->tverts[v1].y + 0.5);

    tu1 = r1_base;
    tv1 = g1_base;
    tw1 = b1_base;

    x2 = (int)(poly->tverts[v2].x + 0.5);
    y2 = (int)(poly->tverts[v2].y + 0.5);

    tu2 = r2_base;
    tv2 = g2_base;
    tw2 = b2_base;

    //设置插值重新开始的值
    yrestart = y1;

    if (tri_type & TRI_TYPE_FLAT_MASK)
    {
        if (tri_type == TRI_TYPE_FLAT_TOP)
        {
            dy = y2 - y0;
            if (dy == 0) return;
            dxdyl = ((x2 - x0) << FIXP16_SHIFT) / dy;
            dudyl = ((tu2 - tu0) << FIXP16_SHIFT) / dy;
            dvdyl = ((tv2 - tv0) << FIXP16_SHIFT) / dy;
            dwdyl = ((tw2 - tw0) << FIXP16_SHIFT) / dy;

            dxdyr = ((x2 - x1) << FIXP16_SHIFT) / dy;
            dudyr = ((tu2 - tu1) << FIXP16_SHIFT) / dy;
            dvdyr = ((tv2 - tv1) << FIXP16_SHIFT) / dy;
            dwdyr = ((tw2 - tw1) << FIXP16_SHIFT) / dy;

            //检测y轴裁剪
            if (y0 < min_clip_y)
            {
                dy = min_clip_y - y0;

                //计算新的LHS开始的值
                xl = dxdyl * dy + x0 << FIXP16_SHIFT;
                ul = dudyl * dy + tu0 << FIXP16_SHIFT;
                vl = dvdyl * dy + tv0 << FIXP16_SHIFT;
                wl = dwdyl * dy + tw0 << FIXP16_SHIFT;

                //计算新的RHS 开始的值
                xr = dxdyr * dy + x1 << FIXP16_SHIFT;
                ur = dudyr * dy + tu1 << FIXP16_SHIFT;
                vr = dvdyr * dy + tv1 << FIXP16_SHIFT;
                wr = dwdyr * dy + tw1 << FIXP16_SHIFT;

                ystart = min_clip_y;
            }
            else //没有裁剪
            {
                xr = x1 << FIXP16_SHIFT;
                ur = tu1 << FIXP16_SHIFT;
                wr = tw1 << FIXP16_SHIFT;
                vr = tv1 << FIXP16_SHIFT;

                xl = x0 << FIXP16_SHIFT;
                ul = tu0 << FIXP16_SHIFT;
                vl = tv0 << FIXP16_SHIFT;
                wl = tw0 << FIXP16_SHIFT;

                ystart = y0;
            }
        }
        else //不是flat top，那么就是flat bottom
        {
            dy = y1 - y0;
            if (dy == 0) return;

            dxdyl = ((x1 - x0) << FIXP16_SHIFT) / dy;
            dudyl = ((tu1 - tu0) << FIXP16_SHIFT) / dy;
            dvdyl = ((tv1 - tv0) << FIXP16_SHIFT) / dy;
            dwdyl = ((tw1 - tw0) << FIXP16_SHIFT) / dy;

            dxdyr = ((x2 - x0) << FIXP16_SHIFT) / dy;
            dudyr = ((tu2 - tu0) << FIXP16_SHIFT) / dy;
            dvdyr = ((tv2 - tv0) << FIXP16_SHIFT) / dy;
            dwdyr = ((tw2 - tw0) << FIXP16_SHIFT) / dy;

            if (y0 < min_clip_y)
            {
                dy = min_clip_y - y0;

                xl = dxdyl*dy + x0 << FIXP16_SHIFT;
                ul = dudyl * dy + tu0 << FIXP16_SHIFT;
                vl = dvdyl * dy + tv0 << FIXP16_SHIFT;
                wl = dvdyl * dy + tw0 << FIXP16_SHIFT;

                xr = dxdyr * dy + x0 << FIXP16_SHIFT;
                ur = dudyr * dy + tu0 << FIXP16_SHIFT;
                vr = dvdyr * dy + tv0 << FIXP16_SHIFT;
                wr = dwdyr * dy + tw0 << FIXP16_SHIFT;

                ystart = min_clip_y;
            }
            else
            {
                xl = xr = x0 << FIXP16_SHIFT;
                ul = ur = tu0 << FIXP16_SHIFT;
                vl = vr = tv0 << FIXP16_SHIFT;
                wl = wr = tw0 << FIXP16_SHIFT;

                ystart = y0;
            }
        }

        if ((yend = y2) > max_clip_y)
            yend = max_clip_y;

        //检测是否有水平裁剪
        if ((x0 < min_clip_x) || (x0 > max_clip_x) || (x1 < min_clip_x) || (x1 > max_clip_x) || (x2 <min_clip_x) || (x2 > max_clip_x))
        {
            screen_ptr = dest_buffer + (ystart * mempitch);

            for (yi = 0; yi <= yend; yi++)
            {
                xstart = (xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT;
                xend = (xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT;

                ui = ul + FIXP16_ROUND_UP;
                wi = wl + FIXP16_ROUND_UP;
                vi = vl + FIXP16_ROUND_UP;
                //计算u,v插值
                if ((dx = (xend - xstart)) > 0)
                {
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    ui += dx*du;
                    vi += dx*dv;
                    dw += dx*dw;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for ( xi = xstart; xi <= xend; xi++)
                {
                    screen_ptr[xi] = (ui >> (3 + FIXP16_SHIFT)) << 11 + (vi >> (2 + FIXP16_SHIFT)) << 5 + wi >> (3 + FIXP16_SHIFT);
                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                //沿着左边和右边插值计算u,v,w,x
                xl += dxdyl;
                ul += dudyl;
                vl += dvdyl;
                wl += dwdyl;

                xr += dxdyr;
                vr += dvdyr;
                wr += dwdyr;
                ur += dudyr;

                //移动到下一行
                screen_ptr += mempitch;
            }
        }
        else
        {
            screen_ptr = dest_buffer + ystart * mempitch;
            for ( yi = ystart; yi <= yend; yi++)
            {
                xstart = (xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT;
                xend = (xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT;

                ui = ul + FIXP16_ROUND_UP;
                vi = vl + FIXP16_ROUND_UP;
                wi = wl + FIXP16_ROUND_UP;

                //计算u,v插值
                if ((dx = xend - xstart) > 0)
                {
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                for ( xi = xstart; xi <= xend; xi++)
                {
                    screen_ptr[xi] = ((ui >> (3 + FIXP16_SHIFT)) << 11) + ((vi >> (2 + FIXP16_SHIFT)) << 5) + (wi >> (3 + FIXP16_SHIFT));

                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                xl += dxdyl;
                ul += dudyl;
                vl += dvdyl;
                wl += dwdyl;

                xr += dxdyr;
                ur += dudyr;
                vr += dvdyr;
                wr += dwdyr;

                screen_ptr += mempitch;
            }
        }

    }
    else if (tri_type == TRI_TYPE_GENERAL)
    {
        if ((yend = y2) > max_clip_y)
            yend = max_clip_y;

        if (y1 < min_clip_y) //转折点被裁剪的情况
        {
            dyl = y2 - y1;

            dxdyl = ((x2 - x1) << FIXP16_SHIFT) / dyl;
            dudyl = ((tu2 - tu1) << FIXP16_SHIFT) / dyl;
            dvdyl = ((tv2 - tv1) << FIXP16_SHIFT) / dyl;
            dwdyl = ((tw2 - tw1) << FIXP16_SHIFT) / dyl;

            dyr = y2 - y0;

            dxdyr = ((x2 - x0) << FIXP16_SHIFT) / dyr;
            dudyr = ((tu2 - tu0) << FIXP16_SHIFT) / dyr;
            dvdyr = ((tv2 - tv0) << FIXP16_SHIFT) / dyr;
            dwdyr = ((tw2 - tw0) << FIXP16_SHIFT) / dyr;

            dyr = min_clip_y - y0;
            dyl = min_clip_y - y1;

            xl = dxdyl * dyl + (x1 << FIXP16_SHIFT);
            ul = dudyl *dyl + (tu1 << FIXP16_SHIFT);
            vl = dvdyl * dyl + (tv1 << FIXP16_SHIFT);
            wl = dwdyl * dyl + (tw1 << FIXP16_SHIFT);

            xr = dxdyr * dyr + (x0 << FIXP16_SHIFT);
            ur = dudyr * dyr + (tu0 << FIXP16_SHIFT);
            vr = dvdyr * dyr + (tv0 << FIXP16_SHIFT);
            wr = dwdyr * dyr + (tw0 << FIXP16_SHIFT);

            ystart = min_clip_y;

            //测试是否需要交换左右的
            if (dxdyr < dxdyl)
            {
                SWAP(dxdyl, dxdyr, tmp);
                SWAP(dudyl, dudyr, tmp);
                SWAP(dvdyl, dvdyr, tmp);
                SWAP(dwdyl, dwdyr, tmp);

                SWAP(xl, xr, tmp);
                SWAP(vl, vr, tmp);
                SWAP(ul, ur, tmp);
                SWAP(wl, wr, tmp);

                SWAP(x1, x2, tmp);
                SWAP(y1, y2, tmp);
                SWAP(tv1, tv2, tmp);
                SWAP(tu1, tu2, tmp);
                SWAP(tw1, tw2, tmp);

                //设置插值重启值
                irestart = INTERP_RHS;
            }
        }
        else if (y0 < min_clip_y) //转折点没有被裁剪的情况
        {
            dyl = y1 - y0;

            dxdyl = ((x1 - x0) << FIXP16_SHIFT) / dyl;
            dudyl = ((tu1 - tu0) << FIXP16_SHIFT) / dyl;
            dvdyl = ((tv1 - tv0) << FIXP16_SHIFT) / dyl;
            dwdyl = ((tw1 - tw0) << FIXP16_SHIFT) / dyl;

            dyr = y2 - y0;

            dxdyr = ((x2 - x0) << FIXP16_SHIFT) / dyr;
            dudyr = ((tu2 - tu0) << FIXP16_SHIFT) / dyr;
            dvdyr = ((tv2 - tv0) << FIXP16_SHIFT) / dyr;
            dwdyr = ((tw2 - tw0) << FIXP16_SHIFT) / dyr;

            //计算被裁减掉的
            dy = min_clip_y - y0;

            xl = dxdyl * dy + (x0 << FIXP16_SHIFT);
            ul = dudyl * dy + (tu0 << FIXP16_SHIFT);
            vl = dvdyl * dy + (tv0 << FIXP16_SHIFT);
            wl = dwdyl * dy + (tw0 << FIXP16_SHIFT);

            xr = dxdyr * dy + (x0 << FIXP16_SHIFT);
            ur = dudyr * dy + (tu0 << FIXP16_SHIFT);
            vr = dvdyr * dy + (tv0 << FIXP16_SHIFT);
            wr = dwdyr * dy + (tw0 << FIXP16_SHIFT);

            ystart = min_clip_y;


            //测试是否需要交换左右的
            if (dxdyr < dxdyl)
            {
                SWAP(dxdyl, dxdyr, tmp);
                SWAP(dudyl, dudyr, tmp);
                SWAP(dvdyl, dvdyr, tmp);
                SWAP(dwdyl, dwdyr, tmp);

                SWAP(xl, xr, tmp);
                SWAP(vl, vr, tmp);
                SWAP(ul, ur, tmp);
                SWAP(wl, wr, tmp);

                SWAP(x1, x2, tmp);
                SWAP(y1, y2, tmp);
                SWAP(tv1, tv2, tmp);
                SWAP(tu1, tu2, tmp);
                SWAP(tw1, tw2, tmp);

                //设置插值重启值
                irestart = INTERP_RHS;
            }
        }
        else //没有y轴的裁剪
        {
            dyl = y1 - y0;

            dxdyl = ((x1 - x0) << FIXP16_SHIFT) / dyl;
            dudyl = ((tu1 - tu0) << FIXP16_SHIFT) / dyl;
            dvdyl = ((tv1 - tv0) << FIXP16_SHIFT) / dyl;
            dwdyl = ((tw1 - tw0) << FIXP16_SHIFT) / dyl;

            dyr = y2 - y0;

            dxdyr = ((x2 - x0) << FIXP16_SHIFT) / dyr;
            dudyr = ((tu2 - tu0) << FIXP16_SHIFT) / dyr;
            dvdyr = ((tv2 - tv0) << FIXP16_SHIFT) / dyr;
            dwdyr = ((tw2 - tw0) << FIXP16_SHIFT) / dyr;

            xr = xl = x0 << FIXP16_SHIFT;
            ur = ul = tu0 << FIXP16_SHIFT;
            vr = vl = tv0 << FIXP16_SHIFT;
            wr = wl = tw0 << FIXP16_SHIFT;

            ystart = y0;

            //测试是否需要交换左右的
            if (dxdyr < dxdyl)
            {
                SWAP(dxdyl, dxdyr, tmp);
                SWAP(dudyl, dudyr, tmp);
                SWAP(dvdyl, dvdyr, tmp);
                SWAP(dwdyl, dwdyr, tmp);

                SWAP(xl, xr, tmp);
                SWAP(vl, vr, tmp);
                SWAP(ul, ur, tmp);
                SWAP(wl, wr, tmp);

                SWAP(x1, x2, tmp);
                SWAP(y1, y2, tmp);
                SWAP(tv1, tv2, tmp);
                SWAP(tu1, tu2, tmp);
                SWAP(tw1, tw2, tmp);

                //设置插值重启值
                irestart = INTERP_RHS;
            }
        }

        //有水平裁剪的情况
        if (x0 < min_clip_x || x0 > max_clip_x || x1 < min_clip_x || x1 > max_clip_x || x2 < min_clip_x || x2 > max_clip_x)
        {
            screen_ptr = dest_buffer + ystart * mempitch;

            for ( yi = ystart; yi <= yend; yi++)
            {
                xstart = (xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT;
                xend = (xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT;

                ui = ul + FIXP16_ROUND_UP;
                vi = vl + FIXP16_ROUND_UP;
                wi = wl + FIXP16_ROUND_UP;

                if ((dx = xend - xstart) > 0)
                {
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    ui += dx*du;
                    vi += dx*dv;
                    wi += dx*dw;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for ( xi = xstart; xi <= xend; xi++)
                {
                    screen_ptr[xi] = ((ui >> (3 + FIXP16_SHIFT)) << 11) + ((vi >> (2 + FIXP16_SHIFT)) << 5) + (wi >> (3 + FIXP16_SHIFT));

                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                xl += dxdyl;
                ul += dudyl;
                wl += dwdyl;
                vl += dvdyl;

                xr += dxdyr;
                ur += dudyr;
                vr += dvdyr;
                wr += dwdyr;

                screen_ptr += mempitch;

                //检测是否到了转折点，如果是，那么改变插值
                if (yi == yrestart)
                {
                    if (irestart == INTERP_LHS) //左手系的三角形
                    {
                        dyl = y2 - y1;

                        dxdyl = ((x2 - x1) <<FIXP16_SHIFT) / dyl;
                        dudyl = ((tu2 - tu1) << FIXP16_SHIFT) / dyl;
                        dwdyl = ((tw2 - tw1) << FIXP16_SHIFT) / dyl;
                        dvdyl = ((tv2 - tv1) << FIXP16_SHIFT) / dyl;

                        xl = x1 << FIXP16_SHIFT;
                        ul = tu1 << FIXP16_SHIFT;
                        vl = tv1 << FIXP16_SHIFT;
                        wl = tw1 << FIXP16_SHIFT;

                        xl += dxdyl;
                        ul += dudyl;
                        vl += dvdyl;
                        wl += dwdyl;
                    }
                    else //右手系的三角形
                    {
                        dyr = y1 - y2;

                        dxdyr = ((x1 - x2) << FIXP16_SHIFT) / dyr;
                        dudyr = ((tu1 - tu2) << FIXP16_SHIFT) / dyr;
                        dvdyr = ((tv1 - tv2) << FIXP16_SHIFT) / dyr;
                        dwdyr = ((tw1 - tw2) << FIXP16_SHIFT) / dyr;

                        xr = x2 << FIXP16_SHIFT;
                        ur = tu2 << FIXP16_SHIFT;
                        vr = tv2 << FIXP16_SHIFT;
                        wr = tw2 << FIXP16_SHIFT;

                        xr += dxdyr;
                        ur += dudyr;
                        wr += dwdyr;
                        vr += dvdyr;
                    }
                }
            }
        }
        else
        {
            screen_ptr = dest_buffer + ystart * mempitch;

            for ( yi = ystart; yi <= yend; yi++)
            {
                xstart = (xl + FIXP16_ROUND_UP) >> FIXP16_SHIFT;
                xend = (xr + FIXP16_ROUND_UP) >> FIXP16_SHIFT;

                ui = ul + FIXP16_ROUND_UP;
                vi = vl + FIXP16_ROUND_UP;
                wi = wl + FIXP16_ROUND_UP;

                if ((dx = xend - xstart) > 0)
                {
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                for ( xi = xstart; xi <= xend; xi++)
                {
                    screen_ptr[xi] = ((ui >> (3 + FIXP16_SHIFT)) << 11) + ((vi >> (2 + FIXP16_SHIFT)) << 5) + (wi >> (3 + FIXP16_SHIFT));

                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                xl += dxdyl;
                ul += dudyl;
                vl += dvdyl;
                wl += dwdyl;

                xr += dxdyr;
                ur += dudyr;
                wr += dwdyr;
                vr += dvdyr;

                screen_ptr += mempitch;

                //检测是否到了转折点，如果是，那么改变插值
                if (yi == yrestart)
                {
                    if (irestart == INTERP_LHS) //左手系的三角形
                    {
                        dyl = y2 - y1;

                        dxdyl = ((x2 - x1) << FIXP16_SHIFT) / dyl;
                        dudyl = ((tu2 - tu1) << FIXP16_SHIFT) / dyl;
                        dwdyl = ((tw2 - tw1) << FIXP16_SHIFT) / dyl;
                        dvdyl = ((tv2 - tv1) << FIXP16_SHIFT) / dyl;

                        xl = x1 << FIXP16_SHIFT;
                        ul = tu1 << FIXP16_SHIFT;
                        vl = tv1 << FIXP16_SHIFT;
                        wl = tw1 << FIXP16_SHIFT;

                        xl += dxdyl;
                        ul += dudyl;
                        vl += dvdyl;
                        wl += dwdyl;
                    }
                    else //右手系的三角形
                    {
                        dyr = y1 - y2;

                        dxdyr = ((x1 - x2) << FIXP16_SHIFT) / dyr;
                        dudyr = ((tu1 - tu2) << FIXP16_SHIFT) / dyr;
                        dvdyr = ((tv1 - tv2) << FIXP16_SHIFT) / dyr;
                        dwdyr = ((tw1 - tw2) << FIXP16_SHIFT) / dyr;

                        xr = x2 << FIXP16_SHIFT;
                        ur = tu2 << FIXP16_SHIFT;
                        vr = tv2 << FIXP16_SHIFT;
                        wr = tw2 << FIXP16_SHIFT;

                        xr += dxdyr;
                        ur += dudyr;
                        wr += dwdyr;
                        vr += dvdyr;
                    }
                }
            }
        }
    }
}

//绘制Gouraud模式的三角形
void Draw_Gouraud_Triangle_Float(Poly4D_PTR poly, UCHAR *_dest_buffer, int mempitch, UCHAR* _z_buffer, int zpitch)
{
    int v0 = 0, v1 = 1, v2 = 2;
    float ftmp;
    int itmp;
    int tri_type;

    //提取的每个顶点的基础颜色
    int r0_base, g0_base, b0_base,
        r1_base, g1_base, b1_base,
        r2_base, g2_base, b2_base;

    //y轴开始和结束的位置
    int yrestart, ystart, yend, xstart, xend;

    float x0, y0, x1, y1, x2, y2,z0,z1,z2,
        tv0, tu0, tw0, tv1, tu1, tw1, tv2, tu2, tw2;
    float dy;
    float dxdyl, dudyl, dvdyl, dwdyl, dxdyr, dudyr, dvdyr, dwdyr,dzdyl,dzdyr;
    float xl, vl, ul, wl, xr, vr, ur, wr,zr,zl;
    float dx, du, dv, dw,dz;
    float ui, vi, wi;
    int xi, yi;
    float zi;
    int tmpu, tmpv, tmpw;

    float dyl, dyr;
    int irestart = INTERP_LHS;

    UINT *screen_ptr = NULL,
        *screen_line = NULL,
        *textmap = NULL,
        *dest_buffer = (UINT *)_dest_buffer;
    float* zbuffer = (float*) _z_buffer,
            *z_ptr = NULL;

    //以32位长度重新计算mempitch
    mempitch = (mempitch >> 2);
    zpitch = (zpitch >> 2);

    //首先进行裁剪测试
    if ((poly->tverts[v0].y < min_clip_y && poly->tverts[v1].y < min_clip_y && poly->tverts[v2].y < min_clip_y) ||
        (poly->tverts[v0].y > max_clip_y && poly->tverts[v1].y > max_clip_y && poly->tverts[v2].y > max_clip_y) ||
        (poly->tverts[v0].x < min_clip_x && poly->tverts[v1].x < min_clip_x && poly->tverts[v2].x < min_clip_x) ||
        (poly->tverts[v0].x > max_clip_x && poly->tverts[v1].x > max_clip_x && poly->tverts[v2].x > max_clip_x))
        return;

    //检测三角形是否退化为直线
    if ((poly->tverts[v0].x == poly->tverts[v1].x && poly->tverts[v1].x == poly->tverts[v2].x) || (poly->tverts[v0].y == poly->tverts[v1].y && poly->tverts[v1].y == poly->tverts[v2].y))
        return;

    //对顶点进行排序
    if (poly->tverts[v1].y < poly->tverts[v0].y)
        SWAP(v0, v1, itmp);
    if (poly->tverts[v2].y < poly->tverts[v0].y)
        SWAP(v2, v0, itmp);
    if (poly->tverts[v2].y < poly->tverts[v1].y)
        SWAP(v2, v1, itmp);

    if ((int)(poly->tverts[v0].y + 0.5) == (int)(poly->tverts[v1].y + 0.5))
    {
        tri_type = TRI_TYPE_FLAT_TOP;
        if (poly->tverts[v0].x > poly->tverts[v1].x)
            SWAP(v0, v1, itmp);
    }
    else if ((int)(poly->tverts[v1].y + 0.5) == (int)(poly->tverts[v2].y + 0.5))
    {
        tri_type = TRI_TYPE_FLAT_BOTTOM;
        if (poly->tverts[v1].x > poly->tverts[v2].x)
            SWAP(v1, v2, itmp);
    }
    else
        tri_type = TRI_TYPE_GENERAL;

    _RGBFROM32BIT(poly->lit_color[v0], &r0_base, &g0_base, &b0_base);
    _RGBFROM32BIT(poly->lit_color[v1], &r1_base, &g1_base, &b1_base);
    _RGBFROM32BIT(poly->lit_color[v2], &r2_base, &g2_base, &b2_base);

    //_RGB555FROM16BIT(poly->lit_color[v0], &r0_base, &g0_base, &b0_base);
    //_RGB555FROM16BIT(poly->lit_color[v1], &r1_base, &g1_base, &b1_base);
    //_RGB555FROM16BIT(poly->lit_color[v2], &r2_base, &g2_base, &b2_base);

    //r0_base <<= 3;
    //g0_base <<= 2;
    //b0_base <<= 3;

    //r1_base <<= 3;
    //g1_base <<= 2;
    //b1_base <<= 3;

    //r2_base <<= 3;
    //g2_base <<= 2;
    //b2_base <<= 3;

    //
    x0 = poly->tverts[v0].x;
    y0 = poly->tverts[v0].y;
    z0 = poly->tverts[v0].z;

    tu0 = r0_base;
    tv0 = g0_base;
    tw0 = b0_base;

    x1 = poly->tverts[v1].x;
    y1 = poly->tverts[v1].y;
    z1 = poly->tverts[v1].z;

    tu1 = r1_base;
    tv1 = g1_base;
    tw1 = b1_base;

    x2 = poly->tverts[v2].x;
    y2 = poly->tverts[v2].y;
    z2 = poly->tverts[v2].z;

    tu2 = r2_base;
    tv2 = g2_base;
    tw2 = b2_base;
//    if(r2_base != 255)
//    {
//        qDebug() << "color: " << r2_base;
//    }

    //设置插值重新开始的值
    yrestart = y1;

    if (tri_type & TRI_TYPE_FLAT_MASK)
    {
        if (tri_type == TRI_TYPE_FLAT_TOP)
        {
            dy = y2 - y0;
            if (dy == 0) return;
            dxdyl = (x2 - x0) / dy;
            dzdyl = (z2 - z0) / dy;
            dudyl = (tu2 - tu0) / dy;
            dvdyl = (tv2 - tv0) / dy;
            dwdyl = (tw2 - tw0) / dy;

            dxdyr = (x2 - x1) / dy;
            dzdyr = (z2 - z1) / dy;
            dudyr = (tu2 - tu1) / dy;
            dvdyr = (tv2 - tv1) / dy;
            dwdyr = (tw2 - tw1) / dy;

            //检测y轴裁剪
            if (y0 < min_clip_y)
            {
                dy = min_clip_y - y0;

                //计算新的LHS开始的值
                xl = dxdyl * dy + x0 ;
                zl = dzdyl * dy + z0;
                ul = dudyl * dy + tu0;
                vl = dvdyl * dy + tv0;
                wl = dwdyl * dy + tw0;

                //计算新的RHS 开始的值
                xr = dxdyr * dy + x1;
                zr = dzdyr * dy + z1;
                ur = dudyr * dy + tu1;
                vr = dvdyr * dy + tv1;
                wr = dwdyr * dy + tw1;

                ystart = min_clip_y;
            }
            else //没有裁剪
            {
                xr = x1;
                zr = z1;
                ur = tu1;
                wr = tw1;
                vr = tv1;

                xl = x0;
                zl = z0;
                ul = tu0;
                vl = tv0;
                wl = tw0;

                ystart = y0;
            }
        }
        else //不是flat top，那么就是flat bottom
        {
            dy = y1 - y0;
            if (dy == 0) return;

            dxdyl = (x1 - x0) / dy;
            dzdyl = (z1 - z0) / dy;
            dudyl = (tu1 - tu0) / dy;
            dvdyl = (tv1 - tv0) / dy;
            dwdyl = (tw1 - tw0) / dy;

            dxdyr = (x2 - x0) / dy;
            dzdyr = (z2 - z0) / dy;
            dudyr = (tu2 - tu0)/ dy;
            dvdyr = (tv2 - tv0) / dy;
            dwdyr = (tw2 - tw0) / dy;

            if (y0 < min_clip_y)
            {
                dy = min_clip_y - y0;

                xl = dxdyl * dy + x0;
                zl = dzdyl * dy + z0;
                ul = dudyl * dy + tu0;
                vl = dvdyl * dy + tv0;
                wl = dwdyl * dy + tw0;

                xr = dxdyr * dy + x0;
                zr = dzdyr * dy + z0;
                ur = dudyr * dy + tu0;
                vr = dvdyr * dy + tv0;
                wr = dwdyr * dy + tw0;

                ystart = min_clip_y;
            }
            else
            {
                xl = xr = x0;
                zl = zr = z0;
                ul = ur = tu0;
                vl = vr = tv0;
                wl = wr = tw0;

                ystart = y0;
            }
        }

        if ((yend = y2) > max_clip_y)
            yend = max_clip_y;

        //检测是否有水平裁剪
        if ((x0 < min_clip_x) || (x0 > max_clip_x) || (x1 < min_clip_x) || (x1 > max_clip_x) || (x2 <min_clip_x) || (x2 > max_clip_x))
        {
            screen_ptr = dest_buffer + (ystart * mempitch);
            z_ptr = zbuffer + ystart * zpitch;

            for (yi = ystart; yi <= yend; yi++)
            {
                xstart = xl;
                xend = xr;

                zi = zl;
                ui = ul;
                wi = wl;
                vi = vl;
                //计算u,v插值
                if ((dx = (xend - xstart)) > 0)
                {
                    dz = (zr - zl) / dx;
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    dz = zr - zl;
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    zi += dx*dz;
                    ui += dx*du;
                    vi += dx*dv;
                    dw += dx*dw;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for (xi = xstart; xi <= xend; xi++)
                {
                    if(zi > z_ptr[xi])
                    {
                        tmpu = ui;
                        tmpv = vi;
                        tmpw = wi;
                        screen_ptr[xi] = (255 << 24) + (tmpu << 16) + (tmpv << 8) + tmpw;
                        z_ptr[xi] = zi;
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                //沿着左边和右边插值计算u,v,w,x
                xl += dxdyl;
                zl += dzdyl;
                ul += dudyl;
                vl += dvdyl;
                wl += dwdyl;

                xr += dxdyr;
                zr += dzdyr;
                vr += dvdyr;
                wr += dwdyr;
                ur += dudyr;

                //移动到下一行
                screen_ptr += mempitch;
                z_ptr += zpitch;
            }
        }
        else
        {
            screen_ptr = dest_buffer + ystart * mempitch;
            z_ptr = zbuffer + ystart * zpitch;
            for (yi = ystart; yi < yend; yi++)
            {
                xstart = xl;
                xend = xr;

                zi = zl;
                ui = ul;
                vi = vl;
                wi = wl;

                //计算u,v插值
                if ((dx = xend - xstart) > 0)
                {
                    dz = (zr - zl) / dx;
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    dz = zr - zl;
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                for (xi = xstart; xi <= xend; xi++)
                {
                    if(zi > z_ptr[xi])
                    {
                        tmpu = ui;
                        tmpv = vi;
                        tmpw = wi;
                        screen_ptr[xi] = (255 << 24) + (tmpu << 16) + (tmpv << 8) + tmpw;
                        z_ptr[xi] = zi;
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                xl += dxdyl;
                zl += dzdyl;
                ul += dudyl;
                vl += dvdyl;
                wl += dwdyl;

                xr += dxdyr;
                zr += dzdyr;
                ur += dudyr;
                vr += dvdyr;
                wr += dwdyr;

                screen_ptr += mempitch;
                z_ptr += zpitch;
            }
        }

    }
    else if (tri_type == TRI_TYPE_GENERAL)
    {
        if ((yend = y2) > max_clip_y)
            yend = max_clip_y;

        if (y1 < min_clip_y) //转折点被裁剪的情况
        {
            dyl = y2 - y1;

            dxdyl = (x2 - x1) / dyl;
            dzdyl = (z2 - z1) / dyl;
            dudyl = (tu2 - tu1) / dyl;
            dvdyl = (tv2 - tv1) / dyl;
            dwdyl = (tw2 - tw1) / dyl;

            dyr = y2 - y0;

            dxdyr = (x2 - x0) / dyr;
            dzdyr = (z2 - z0) / dyr;
            dudyr = (tu2 - tu0) / dyr;
            dvdyr = (tv2 - tv0) / dyr;
            dwdyr = (tw2 - tw0) / dyr;

            dyr = min_clip_y - y0;
            dyl = min_clip_y - y1;

            xl = dxdyl * dyl + x1;
            zl = dzdyl * dyl + z1;
            ul = dudyl * dyl + tu1;
            vl = dvdyl * dyl + tv1;
            wl = dwdyl * dyl + tw1;

            xr = dxdyr * dyr + x0;
            zr = dzdyr * dyr + z0;
            ur = dudyr * dyr + tu0;
            vr = dvdyr * dyr + tv0;
            wr = dwdyr * dyr + tw0;

            ystart = min_clip_y;

            //测试是否需要交换左右的
            if (dxdyr < dxdyl)
            {
                SWAP(dxdyl, dxdyr, ftmp);
                SWAP(dzdyl, dzdyr, ftmp);
                SWAP(dudyl, dudyr, ftmp);
                SWAP(dvdyl, dvdyr, ftmp);
                SWAP(dwdyl, dwdyr, ftmp);

                SWAP(xl, xr, ftmp);
                SWAP(zl, zr, ftmp);
                SWAP(vl, vr, ftmp);
                SWAP(ul, ur, ftmp);
                SWAP(wl, wr, ftmp);

                SWAP(x1, x2, ftmp);
                SWAP(y1, y2, ftmp);
                SWAP(z1, z2, ftmp);
                SWAP(tv1, tv2, ftmp);
                SWAP(tu1, tu2, ftmp);
                SWAP(tw1, tw2, ftmp);

                //设置插值重启值
                irestart = INTERP_RHS;
            }
        }
        else if (y0 < min_clip_y) //转折点没有被裁剪的情况
        {
            dyl = y1 - y0;

            dxdyl = (x1 - x0) / dyl;
            dzdyl = (z1 - z0) / dyl;
            dudyl = (tu1 - tu0) / dyl;
            dvdyl = (tv1 - tv0) / dyl;
            dwdyl = (tw1 - tw0) / dyl;

            dyr = y2 - y0;

            dxdyr = (x2 - x0) / dyr;
            dzdyr = (z2 - z0) / dyr;
            dudyr = (tu2 - tu0) / dyr;
            dvdyr = (tv2 - tv0) / dyr;
            dwdyr = (tw2 - tw0) / dyr;

            //计算被裁减掉的
            dy = min_clip_y - y0;

            xl = dxdyl * dy + x0;
            zl = dzdyl * dy + z0;
            ul = dudyl * dy + tu0;
            vl = dvdyl * dy + tv0;
            wl = dwdyl * dy + tw0;

            xr = dxdyr * dy + x0;
            zr = dzdyr * dy + z0;
            ur = dudyr * dy + tu0;
            vr = dvdyr * dy + tv0;
            wr = dwdyr * dy + tw0;

            ystart = min_clip_y;


            //测试是否需要交换左右的
            if (dxdyr < dxdyl)
            {
                SWAP(dxdyl, dxdyr, ftmp);
                SWAP(dzdyl, dzdyr, ftmp);
                SWAP(dudyl, dudyr, ftmp);
                SWAP(dvdyl, dvdyr, ftmp);
                SWAP(dwdyl, dwdyr, ftmp);

                SWAP(xl, xr, ftmp);
                SWAP(zl, zr, ftmp);
                SWAP(vl, vr, ftmp);
                SWAP(ul, ur, ftmp);
                SWAP(wl, wr, ftmp);

                SWAP(x1, x2, ftmp);
                SWAP(y1, y2, ftmp);
                SWAP(z1, z2, ftmp);
                SWAP(tv1, tv2, ftmp);
                SWAP(tu1, tu2, ftmp);
                SWAP(tw1, tw2, ftmp);

                //设置插值重启值
                irestart = INTERP_RHS;
            }
        }
        else //没有y轴的裁剪
        {
            dyl = y1 - y0;

            dxdyl = (x1 - x0) / dyl;
            dzdyl = (z1 - z0) / dyl;
            dudyl = (tu1 - tu0) / dyl;
            dvdyl = (tv1 - tv0) / dyl;
            dwdyl = (tw1 - tw0) / dyl;

            dyr = y2 - y0;

            dxdyr = (x2 - x0) / dyr;
            dzdyr = (z2 - z0) / dyr;
            dudyr = (tu2 - tu0) / dyr;
            dvdyr = (tv2 - tv0) / dyr;
            dwdyr = (tw2 - tw0) / dyr;

            xr = xl = x0;
            zr = zl = z0;
            ur = ul = tu0;
            vr = vl = tv0;
            wr = wl = tw0;

            ystart = y0;

            //测试是否需要交换左右的
            if (dxdyr < dxdyl)
            {
                SWAP(dxdyl, dxdyr, ftmp);
                SWAP(dzdyl, dzdyr, ftmp);
                SWAP(dudyl, dudyr, ftmp);
                SWAP(dvdyl, dvdyr, ftmp);
                SWAP(dwdyl, dwdyr, ftmp);

                SWAP(xl, xr, ftmp);
                SWAP(zl, zr, ftmp);
                SWAP(vl, vr, ftmp);
                SWAP(ul, ur, ftmp);
                SWAP(wl, wr, ftmp);

                SWAP(x1, x2, ftmp);
                SWAP(y1, y2, ftmp);
                SWAP(z1, z2, ftmp);
                SWAP(tv1, tv2, ftmp);
                SWAP(tu1, tu2, ftmp);
                SWAP(tw1, tw2, ftmp);

                //设置插值重启值
                irestart = INTERP_RHS;
            }
        }

        //有水平裁剪的情况
        if (x0 < min_clip_x || x0 > max_clip_x || x1 < min_clip_x || x1 > max_clip_x || x2 < min_clip_x || x2 > max_clip_x)
        {
            screen_ptr = dest_buffer + ystart * mempitch;
            z_ptr = zbuffer + ystart * zpitch;

            for (yi = ystart; yi <= yend; yi++)
            {
                xstart = xl;
                xend = xr;

                zi = zl;
                ui = ul;
                vi = vl;
                wi = wl;

                if ((dx = xend - xstart) > 0)
                {
                    dz = (zr - zl) / dx;
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    dz = zr - zl;
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    zi += dx*dz;
                    ui += dx*du;
                    vi += dx*dv;
                    wi += dx*dw;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for (xi = xstart; xi <= xend; xi++)
                {
                    if(zi > z_ptr[xi])
                    {
                        tmpu = ui;
                        tmpv = vi;
                        tmpw = wi;
                        screen_ptr[xi] = (255 << 24) + (tmpu << 16) + (tmpv << 8) + tmpw;
                        z_ptr[xi] = zi;
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                xl += dxdyl;
                zl += dzdyl;
                ul += dudyl;
                wl += dwdyl;
                vl += dvdyl;

                xr += dxdyr;
                zr += dzdyr;
                ur += dudyr;
                vr += dvdyr;
                wr += dwdyr;

                screen_ptr += mempitch;
                z_ptr += zpitch;

                //检测是否到了转折点，如果是，那么改变插值
                if (yi == yrestart)
                {
                    if (irestart == INTERP_LHS) //左手系的三角形
                    {
                        dyl = y2 - y1;

                        dxdyl = (x2 - x1) / dyl;
                        dzdyl = (z2 - z1) / dyl;
                        dudyl = (tu2 - tu1) / dyl;
                        dwdyl = (tw2 - tw1) / dyl;
                        dvdyl = (tv2 - tv1) / dyl;

                        xl = x1;
                        zl = z1;
                        ul = tu1;
                        vl = tv1;
                        wl = tw1;

                        zl += dzdyl;
                        xl += dxdyl;
                        ul += dudyl;
                        vl += dvdyl;
                        wl += dwdyl;
                    }
                    else //右手系的三角形
                    {
                        dyr = y1 - y2;

                        dxdyr = (x1 - x2) / dyr;
                        dzdyr = (z1 - z2) / dyr;
                        dudyr = (tu1 - tu2) / dyr;
                        dvdyr = (tv1 - tv2) / dyr;
                        dwdyr = (tw1 - tw2) / dyr;

                        xr = x2;
                        zr = z2;
                        ur = tu2;
                        vr = tv2;
                        wr = tw2;

                        xr += dxdyr;
                        zr += dzdyr;
                        ur += dudyr;
                        wr += dwdyr;
                        vr += dvdyr;
                    }
                }
            }
        }
        else
        {
            screen_ptr = dest_buffer + ystart * mempitch;
            z_ptr = zbuffer + ystart * zpitch;

            for (yi = ystart; yi <= yend; yi++)
            {
                xstart = xl;
                xend = xr;

                zi = zl;
                ui = ul;
                vi = vl;
                wi = wl;

                if ((dx = xend - xstart) > 0)
                {
                    dz = (zr - zl) / dx;
                    du = (ur - ul) / dx;
                    dv = (vr - vl) / dx;
                    dw = (wr - wl) / dx;
                }
                else
                {
                    dz = zr - zl;
                    du = ur - ul;
                    dv = vr - vl;
                    dw = wr - wl;
                }

                for (xi = xstart; xi <= xend; xi++)
                {
                    if(zi > z_ptr[xi])
                    {
                        tmpu = ui;
                        tmpv = vi;
                        tmpw = wi;
                        screen_ptr[xi] = (255 << 24) + (tmpu << 16) + (tmpv << 8) + tmpw;
                        //screen_ptr[xi] = 255 + (tmpu << 24) + (tmpv << 16) + (tmpw << 8);
                        z_ptr[xi] = zi;
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                    wi += dw;
                }

                xl += dxdyl;
                zl += dzdyl;
                ul += dudyl;
                vl += dvdyl;
                wl += dwdyl;

                xr += dxdyr;
                zr += dzdyr;
                ur += dudyr;
                wr += dwdyr;
                vr += dvdyr;

                screen_ptr += mempitch;
                z_ptr += zpitch;

                //检测是否到了转折点，如果是，那么改变插值
                if (yi == yrestart)
                {
                    if (irestart == INTERP_LHS) //左手系的三角形
                    {
                        dyl = y2 - y1;

                        dxdyl = (x2 - x1) / dyl;
                        dzdyl = (z2 - z1) / dyl;
                        dudyl = (tu2 - tu1) / dyl;
                        dwdyl = (tw2 - tw1) / dyl;
                        dvdyl = (tv2 - tv1) / dyl;

                        xl = x1;
                        zl = z1;
                        ul = tu1;
                        vl = tv1;
                        wl = tw1;

                        xl += dxdyl;
                        zl += dzdyl;
                        ul += dudyl;
                        vl += dvdyl;
                        wl += dwdyl;
                    }
                    else //右手系的三角形
                    {
                        dyr = y1 - y2;

                        dxdyr = (x1 - x2) / dyr;
                        dzdyr = (z1 - z2) / dyr;
                        dudyr = (tu1 - tu2) / dyr;
                        dvdyr = (tv1 - tv2) / dyr;
                        dwdyr = (tw1 - tw2) / dyr;

                        xr = x2;
                        zr = z2;
                        ur = tu2;
                        vr = tv2;
                        wr = tw2;

                        xr += dxdyr;
                        zr += dzdyr;
                        ur += dudyr;
                        wr += dwdyr;
                        vr += dvdyr;
                    }
                }
            }
        }
    }
}

void Draw_Textured_Triangle(Poly4D_PTR poly, UCHAR *_dest_buffer, int mempitch, UCHAR *_zbuffer, int zpitch)
{
    //这里的start和end都表示的是像素，所以用整数
    int ystart, yend, xstart, xend, yi, xi;
    int yrestart;
    int v0 = 0, v1 = 1, v2 = 2;
    float x0, x1, x2, y0, y1, y2, z0, z1, z2, tu0, tv0, tu1, tv1, tu2, tv2;
    int itmp;
    float ftmp;
    float ui, vi, zi;

    float dxdyl, dudyl, dvdyl, dxdyr, dudyr, dvdyr, dzdyl, dzdyr;
    float xl, xr, zl, zr, tul, tur, tvl, tvr;
    float dy, dx, dz, du, dv, dyl, dyr;

    int tri_type;
    int irestart = INTERP_LHS;

    UINT textel;
    int r_tex, g_tex, b_tex;
    int r_base, g_base, b_base;

    UINT* screen_ptr = NULL;
    UINT *dest_buffer = (UINT *)_dest_buffer;

    float *z_ptr = NULL,
        *zbuffer = (float *)_zbuffer;


    //首先进行裁剪测试
    if ((poly->tverts[v0].y < min_clip_y && poly->tverts[v1].y < min_clip_y && poly->tverts[v2].y < min_clip_y) ||
        (poly->tverts[v0].y > max_clip_y && poly->tverts[v1].y > max_clip_y && poly->tverts[v2].y > max_clip_y) ||
        (poly->tverts[v0].x < min_clip_x && poly->tverts[v1].x < min_clip_x && poly->tverts[v2].x < min_clip_x) ||
        (poly->tverts[v0].x > max_clip_x && poly->tverts[v1].x > max_clip_x && poly->tverts[v2].x > max_clip_x))
        return;

    //检测三角形是否退化为直线
    if ((poly->tverts[v0].x == poly->tverts[v1].x && poly->tverts[v1].x == poly->tverts[v2].x) || (poly->tverts[v0].y == poly->tverts[v1].y && poly->tverts[v1].y == poly->tverts[v2].y))
        return;

    //32位颜色时的内存跨距
    mempitch = mempitch >> 2;
    zpitch = zpitch >> 2;

    //对顶点进行排序
    if (poly->tverts[v1].y < poly->tverts[v0].y)
        SWAP(v1, v0, itmp);
    if (poly->tverts[v2].y < poly->tverts[v0].y)
        SWAP(v2, v0, itmp);
    if (poly->tverts[v2].y < poly->tverts[v1].y)
        SWAP(v1, v2, itmp);

    //检测三角形的类型
    if ((int)(poly->tverts[v1].y + 0.5) == (int)(poly->tverts[v0].y + 0.5))
    {
        if (poly->tverts[v1].x < poly->tverts[v0].x)
        {
            SWAP(v1, v0, itmp);
        }
        tri_type = TRI_TYPE_FLAT_TOP;
    }
    else if ((int)(poly->tverts[v2].y + 0.5) == (int)(poly->tverts[v1].y +0.5))
    {
        if (poly->tverts[v2].x < poly->tverts[v1].x)
        {
            SWAP(v2, v1, itmp);
        }
        tri_type = TRI_TYPE_FLAT_BOTTOM;
    }
    else
    {
        tri_type = TRI_TYPE_GENERAL;
    }

    x0 = poly->tverts[v0].x;
    y0 = poly->tverts[v0].y;
    z0 = poly->tverts[v0].z;
    tu0 = poly->tverts[v0].t.x;
    tv0 = poly->tverts[v0].t.y;

    x1 = poly->tverts[v1].x;
    y1 = poly->tverts[v1].y;
    z1 = poly->tverts[v1].z;
    tu1 = poly->tverts[v1].t.x;
    tv1 = poly->tverts[v1].t.y;

    x2 = poly->tverts[v2].x;
    y2 = poly->tverts[v2].y;
    z2 = poly->tverts[v2].z;
    tu2 = poly->tverts[v2].t.x;
    tv2 = poly->tverts[v2].t.y;


    if (tri_type & TRI_TYPE_FLAT_MASK)
    {
        if (tri_type == TRI_TYPE_FLAT_BOTTOM)
        {
            dy = y2 - y0;

            dxdyl = (x1 - x0) / dy;
            dzdyl = (z1 - z0) / dy;
            dudyl = (tu1 - tu0) / dy;
            dvdyl = (tv1 - tv0) / dy;

            dxdyr = (x2 - x0) / dy;
            dzdyr = (z2 - z0) / dy;
            dudyr = (tu2 - tu0) / dy;
            dvdyr = (tv2 - tv0) / dy;

            xl = x0;
            zl = z0;
            tul = tu0;
            tvl = tv0;

            xr = x0;
            zr = z0;
            tur = tu0;
            tvr = tv0;

            //进行裁剪
            ystart = y0;
            if (ystart < min_clip_y)
            {
                dy = min_clip_y - y0;

                xl += dy * dxdyl;
                zl += dy * dzdyl;
                tul += dy * dudyl;
                tvl += dy * dvdyl;

                xr += dy * dxdyr;
                zr += dy * dzdyr;
                tur += dy * dudyr;
                tvr += dy * dvdyr;

                ystart = min_clip_y;

            }

            yend = y2;
            if (yend > max_clip_y)
            {
                yend = max_clip_y;
            }

            screen_ptr = dest_buffer + (ystart * mempitch);
            z_ptr = zbuffer + ystart * zpitch;

            for ( yi = ystart; yi <= yend; yi++)
            {
                xstart = xl;
                xend = xr;

                if ((dx = xend - xstart) > 0)
                {
                    du = (tur - tul) / dx;
                    dv = (tvr - tvl) / dx;
                    dz = (zr - zl) / dx;
                }
                else
                {
                    du = tur - tul;
                    dv = tvr - tvl;
                    dz = zr - zl;
                }

                //如果有x方向的裁剪
                ui = tul;
                vi = tvl;
                zi = zl;

                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    ui += dx * du;
                    vi += dx * dv;
                    zi += dx * dz;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for (xi = xstart; xi <= xend; xi++)
                {
                    //使用的是1/z缓存
                    if (zi > z_ptr[xi])
                    {
                        int tex_coord_x = abs(((int)(ui*poly->texture->height))*poly->texture->width);
                        if (tex_coord_x > (poly->texture->height - 1) * poly->texture->width)
                        {
                            tex_coord_x = (poly->texture->height - 1) * poly->texture->width;
                        }

                        //根据ui，vi进行采样
                        screen_ptr[xi] = ((UINT *)poly->texture->buffer)[tex_coord_x + abs((int)(vi*poly->texture->width))];
                        z_ptr[xi] = zi;
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                }

                xl += dxdyl;
                tul += dudyl;
                tvl += dvdyl;
                zl += dzdyl;

                xr += dxdyr;
                tur += dudyr;
                tvr += dvdyr;
                zr += dzdyr;

                screen_ptr += mempitch;
                z_ptr += zpitch;
            }

        }
        else //flat top
        {
            dy = y2 - y0;

            dxdyr = (x2 - x1) / dy;
            dzdyr = (z2 - z1) / dy;
            dudyr = (tu2 - tu1) / dy;
            dvdyr = (tv2 - tv1) / dy;

            dxdyl = (x2 - x0) / dy;
            dzdyl = (z2 - z0) / dy;
            dudyl = (tu2 - tu0) / dy;
            dvdyl = (tv2 - tv0) / dy;

            xl = x0;
            tul = tu0;
            tvl = tv0;
            zl = z0;

            xr = x1;
            tur = tu1;
            tvr = tv1;
            zr = z1;

            //进行裁剪
            ystart = y0;
            if (ystart < min_clip_y)
            {
                dy = min_clip_y - y0;

                xl += dy * dxdyl;
                tul += dy * dudyl;
                tvl += dy * dvdyl;
                zl += dy * dzdyl;

                xr += dy * dxdyr;
                tur += dy * dudyr;
                tvr += dy * dvdyr;
                zr += dy * dzdyr;

                ystart = min_clip_y;
            }

            yend = y2;
            if (yend > max_clip_y)
            {
                yend = max_clip_y;
            }

            screen_ptr = dest_buffer + (ystart * mempitch);
            z_ptr = zbuffer + ystart * zpitch;

            for (yi = ystart; yi <= yend; yi++)
            {
                xstart = xl;
                xend = xr;

                if ((dx = xend - xstart) > 0)
                {
                    du = (tur - tul) / dx;
                    dv = (tvr - tvl) / dx;
                    dz = (zr - zl) / dx;
                }
                else
                {
                    du = tur - tul;
                    dv = tvr - tvl;
                    dz = zr - zl;
                }

                //如果有x方向的裁剪
                ui = tul;
                vi = tvl;
                zi = zl;
                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    ui += dx * du;
                    vi += dx * dv;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for (xi = xstart; xi <= xend; xi++)
                {

                    if (zi > z_ptr[xi])
                    {

                        int tex_coord_x = abs(((int)(ui*poly->texture->height))*poly->texture->width);
                        if (tex_coord_x > (poly->texture->height - 1) * poly->texture->width)
                        {
                            tex_coord_x = (poly->texture->height - 1) * poly->texture->width;
                        }

                        //根据ui，vi进行采样
                        screen_ptr[xi] = ((UINT *)poly->texture->buffer)[tex_coord_x + abs((int)(vi*poly->texture->width))];
                        z_ptr[xi] = zi;
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                }

                xl += dxdyl;
                tul += dudyl;
                tvl += dvdyl;
                zl += dzdyl;

                xr += dxdyr;
                tur += dudyr;
                tvr += dvdyr;
                zr += dzdyr;

                screen_ptr += mempitch;
                z_ptr += zpitch;
            }
        }
    }
    else if (tri_type == TRI_TYPE_GENERAL) //一般性三角形
    {
        yend = y2;
        if (yend > max_clip_y)
            yend = max_clip_y;

        if (y1 < min_clip_y) //转折点被裁剪掉
        {
            dyl = y2 - y1;
            dxdyl = (x2 - x1) / dyl;
            dzdyl = (z2 - z1) / dyl;
            dudyl = (tu2 - tu1) / dyl;
            dvdyl = (tv2 - tv1) / dyl;

            dyr = y2 - y0;
            dzdyr = (z2 - z0) / dyr;
            dxdyr = (x2 - x0) / dyr;
            dudyr = (tu2 - tu0) / dyr;
            dvdyr = (tv2 - tv1) / dyr;

            dyl = min_clip_y - y1;
            dyr = min_clip_y - y0;

            xl = x1 + dxdyl * dyl;
            xr = x0 + dxdyr * dyr;
            zl = z1 + dzdyl * dyl;
            zr = z0 + dzdyr * dyr;
            tul = tu1 + dudyl * dyl;
            tur = tu0 + dudyr * dyr;
            tvl = tv1 + dvdyl * dyl;
            tvr = tv0 + dvdyr * dyr;

            ystart = min_clip_y;

            if (dxdyr < dxdyl)
            {
                SWAP(x1, x2, ftmp);
                SWAP(y1, y2, ftmp);
                SWAP(tu1, tu2, ftmp);
                SWAP(tv1, tv2, ftmp);
                SWAP(dxdyr, dxdyl, ftmp);
                SWAP(dudyr, dudyl, ftmp);
                SWAP(dvdyr, dvdyl, ftmp);
                SWAP(xl, xr, ftmp);
                SWAP(tul, tur, ftmp);
                SWAP(tvl, tvr, ftmp);
                SWAP(zl, zr, ftmp);

                irestart = INTERP_RHS;
            }

            screen_ptr = dest_buffer + (ystart * mempitch);
            z_ptr = zbuffer + ystart * zpitch;
            for (yi = ystart; yi < yend; yi++)
            {
                xstart = xl;
                xend = xr;

                if ((dx = xend - xstart) > 0)
                {
                    du = (tur - tul) / dx;
                    dv = (tvr - tvl) / dx;
                    dz = (zr - zl) / dx;
                }
                else
                {
                    du = (tur - tul);
                    dv = (tvr - tvl);
                    dz = zr - zl;
                }

                ui = tul;
                vi = tvl;
                zi = zl;
                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    ui = tul + du*dx;
                    vi = tvl + dv*dx;
                    zi = zl + dz*dx;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for (xi = xstart; xi <= xend; xi++)
                {
                    if (zi > z_ptr[xi])
                    {
                        int tex_coord_x = abs(((int)(ui*poly->texture->height))*poly->texture->width);
                        if (tex_coord_x > (poly->texture->height - 1) * poly->texture->width)


                        //根据ui，vi进行采样
                        screen_ptr[xi] = ((UINT *)poly->texture->buffer)[tex_coord_x + abs((int)(vi*poly->texture->width))];

                        z_ptr[xi] = zi;
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                }

                xl += dxdyl;
                tul += dudyl;
                tvl += dvdyl;
                zl += dzdyl;

                xr += dxdyr;
                tur += dudyr;
                tvr += dvdyr;
                zr += dzdyr;

                screen_ptr += mempitch;
                z_ptr += zpitch;
            }

        }
        else //转折点没有被裁剪掉
        {
            yrestart = y1;

            dyl = y1 - y0;
            dxdyl = (x1 - x0) / dyl;
            dzdyl = (z1 - z0) / dyl;
            dudyl = (tu1 - tu0) / dyl;
            dvdyl = (tv1 - tv0) / dyl;

            dyr = y2 - y0;
            dxdyr = (x2 - x0) / dyr;
            dzdyr = (z2 - z0) / dyr;
            dudyr = (tu2 - tu0) / dyr;
            dvdyr = (tv2 - tv0) / dyr;

            dy = min_clip_y - y0;
            if (dy < 0) dy = 0; //表示没有裁剪

            xl = x0 + dxdyl * dy;
            xr = x0 + dxdyr * dy;
            zl = z0 + dzdyl * dy;
            zr = z0 + dzdyr * dy;
            tul = tu0 + dudyl * dy;
            tur = tu0 + dudyr * dy;
            tvl = tv0 + dvdyl * dy;
            tvr = tv0 + dvdyr * dy;

            ystart = min_clip_y > y0 ? min_clip_y : y0;

            if (dxdyr < dxdyl)
            {
                SWAP(x1, x2, ftmp);
                SWAP(y1, y2, ftmp);
                SWAP(z1, z2, ftmp);
                SWAP(dyr, dyl, ftmp);
                SWAP(tu1, tu2, ftmp);
                SWAP(tv1, tv2, ftmp);
                SWAP(dxdyr, dxdyl, ftmp);
                SWAP(dzdyr, dzdyl, ftmp);

                SWAP(dudyr, dudyl, ftmp);
                SWAP(dvdyr, dvdyl, ftmp);
                SWAP(xl, xr, ftmp);
                SWAP(tul, tur, ftmp);
                SWAP(tvl, tvr, ftmp);
                SWAP(zl, zr, ftmp);

                irestart = INTERP_RHS;
            }

//            float tmpur = tur;
//            float tmpul = tul;

            screen_ptr = dest_buffer + (ystart * mempitch);
            z_ptr = zbuffer + ystart * zpitch;
            for (yi = ystart; yi < yend; yi++)
            {
                xstart = xl;
                xend = xr;

                if ((dx = xend - xstart) > 0)
                {
                    du = (tur - tul) / dx;
                    dv = (tvr - tvl) / dx;
                    dz = (zr - zl) / dx;
                }
                else
                {
                    du = (tur - tul);
                    dv = (tvr - tvl);
                    dz = zr - zl;
                }

                ui = tul;
                vi = tvl;
                zi = zl;
                if (xstart < min_clip_x)
                {
                    dx = min_clip_x - xstart;

                    ui = tul + du*dx;
                    vi = tvl + dv*dx;
                    zi = zl + dz*dx;

                    xstart = min_clip_x;
                }

                if (xend > max_clip_x)
                    xend = max_clip_x;

                for (xi = xstart; xi <= xend; xi++)
                {
                    float tmpz = -1;
                    if (zi > z_ptr[xi])
                    {
                        int tex_coord_x = abs(((int)(ui*poly->texture->height))*poly->texture->width);
                        if (tex_coord_x > (poly->texture->height - 1) * poly->texture->width)
                        {
                            tex_coord_x = (poly->texture->height - 1) * poly->texture->width;
                        }


                        //根据ui，vi进行采样
                        screen_ptr[xi] = ((UINT *)poly->texture->buffer)[tex_coord_x + abs((int)(vi*poly->texture->width))];
//                        unsigned int colorr = screen_ptr[xi] >> 24;
//                        unsigned int colorg = screen_ptr[xi] >> 16 & 0xff;
//                        unsigned int colorb = screen_ptr[xi] >> 8 & 0xff;
//                        unsigned int colora = screen_ptr[xi] & 0xff;

                        z_ptr[xi] = zi;
                    }else
                    {
                        tmpz = z_ptr[xi];
                    }

                    zi += dz;
                    ui += du;
                    vi += dv;
                }

                xl += dxdyl;
                tul += dudyl;
                tvl += dvdyl;
                zl += dzdyl;

                if(zl < 0)
                {
                    zl = 0;
                }

                xr += dxdyr;
                tur += dudyr;
                tvr += dvdyr;
                zr += dzdyr;

                screen_ptr += mempitch;
                z_ptr += zpitch;

                if (yi == yrestart)
                {
                    if (irestart == INTERP_LHS)
                    {
                        dyl = y2 - y1;

                        dxdyl = (x2 - x1) / dyl;
                        dudyl = (tu2 - tu1) / dyl;
                        dvdyl = (tv2 - tv1) / dyl;
                        dzdyl = (z2 - z1) / dyl;

                        tul = tu1;
                        tvl = tv1;
                        xl = x1;
                        zl = z1;

                        xl += dxdyl;
                        tul += dudyl;
                        tvl += dvdyl;
                        zl += dzdyl;

                    }
                    else if (irestart == INTERP_RHS)
                    {
                        dyr = y1 - y2;

                        dxdyr = (x1 - x2) / dyr;
                        dudyr = (tu1 - tu2) / dyr;
                        dvdyr = (tv1 - tv2) / dyr;
                        dzdyr = (z1 - z2) / dyr;

                        tur = tu2;
                        tvr = tv2;
                        xr = x2;
                        zr = z2;

                        xr += dxdyr;
                        tur += dudyr;
                        tvr += dvdyr;
                        zr += dzdyr;
                    }
                }
            }
        }

    }
}
