#include <QGraphicsScene>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsSimpleTextItem>
#include <QPixmap>
#include <QTime>
#include <QtDebug>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>
#include <QDirIterator>

#include <fbx_loader/include/assimp/scene.h>
#include <fbx_loader/include/assimp/Importer.hpp>
#include <fbx_loader/include/assimp/postprocess.h>
#include <fbx_loader/include/assimp/DefaultLogger.hpp>
#include "gamecontroller.h"


GameController::GameController(QGraphicsScene &scene, QObject *parent):QObject(parent),scene(scene),pixmap(new QGraphicsPixmapItem()),
    render_info(new QGraphicsSimpleTextItem("info"))
{

    qDebug()<<"DWORD: " << sizeof(DWORD) << " WORD: " << sizeof(WORD) << "bitmap file header: " << sizeof(BITMAPFILEHEADER) << " float: " << sizeof(float);
    scene.installEventFilter(this);

    Game_Init();
//    LoadPolys();
    LoadModel();

    Game_Main();

    QFile file(QStringLiteral(":/texture/resource/cloud03.bmp"));
    //file.open(QIODevice::ReadOnly);
    if(!file.open(QIODevice::ReadOnly)){
        qDebug() <<"no such file";
    }

//    QDirIterator it(":",QDirIterator::Subdirectories);
//    while(it.hasNext())
//    {
//        qDebug() << it.next();
//    }

    //QTextStream out(&file);
    //out << "adfadsf";
//    file.write("adfadf");

    //file.close();
    //QFileInfo info(file);

    qDebug() << file.size();



    //pixmap->pixmap().fromImage(QImage("../resource/bmp1.bmp"));
    //qDebug()<<"pixmap: " << pixmap->pixmap().width() << "image: " << image.height() << "file: " << file.size();

    scene.addItem(static_cast<QGraphicsItem*>(pixmap));
    scene.addItem(static_cast<QGraphicsItem*>(render_info));
    //timer.

    connect(&timer,SIGNAL(timeout()),&scene,SLOT(advance()));

    //connect(&timer,timer.timeout,this,update);
    connect(&timer,SIGNAL(timeout()),this,SLOT(update()));

    mytime.start();

    timer.start(1000/33);
}

GameController::~GameController()
{
    free(back_buffer);
}

void GameController::update()
{
//    qDebug()<<"update";
    if(is_update)
    {
        Game_Main();
    }
//
}

void GameController::handleKeyPressed(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
//        qDebug()<<"key left";

        ang_y += ang_y_delta;
        Game_Main();
        //Game_Main();
        break;
    case Qt::Key_Right:

        ang_y -= ang_y_delta;
        Game_Main();
        break;
    case Qt::Key_Up:
//        ang_y_delta+=0.01;
//        curr_cam->pos.z +=1;
        if(ang_y > 0)
        {
            ang_y = - ang_y;
        }
        Game_Main();
        break;
    case Qt::Key_Down:
//        ang_y_delta-=0.01;
//        curr_cam->pos.z -=1;
        if(ang_y < 0)
        {
            ang_y = - ang_y;
        }
        Game_Main();
        break;
    case Qt::Key_S:
        is_update = !is_update;
        break;
    case Qt::Key_C:
        if(curr_cam == &cam)
        {
            curr_cam = &cam1;
        }else{
            curr_cam = &cam;
        }
        break;
    case Qt::Key_F:
        if(render_state & RENDER_STATE_FRAME)
        {
            render_state &= ~RENDER_STATE_FRAME;
            SET_BIT(render_state, RENDER_STATE_SOLID);
        }else{
            render_state &= ~RENDER_STATE_SOLID;
            SET_BIT(render_state, RENDER_STATE_FRAME);
        }
        break;
    case Qt::Key_L:
        if(render_state & RENDER_STATE_LIGHT)
        {
            render_state &= ~RENDER_STATE_LIGHT;
        }else{
            SET_BIT(render_state,RENDER_STATE_LIGHT);
        }
        break;
    case Qt::Key_B:
        if(render_state & RENDER_STATE_REMOVE_BACK)
        {
            render_state &= ~RENDER_STATE_REMOVE_BACK;
        }else{
            SET_BIT(render_state,RENDER_STATE_REMOVE_BACK);
        }
        break;
    case Qt::Key_Z:
        if(render_state & RENDER_STATE_ZBUFFER)
        {
            render_state &= ~RENDER_STATE_ZBUFFER;
        }else{
            SET_BIT(render_state,RENDER_STATE_ZBUFFER);
        }
        break;
    }

}

void GameController::handleMouseClick(QMouseEvent *event)
{
    switch(event->button())
    {
    case Qt::MidButton:

        break;
    case Qt::LeftButton:
        break;
    case Qt::RightButton:
        break;
    }
}

void GameController::handleMouseWheel(QGraphicsSceneWheelEvent *event)
{
//    qDebug() << "delta: " << (float)event->delta() / 10;
//    QPoint tmp = event->delta();
    curr_cam->pos.z += ((float)event->delta() / 10);
}

void GameController::handleMouseMove(QGraphicsSceneMouseEvent *event)
{
//    event->
}

bool GameController::eventFilter(QObject *object, QEvent *event)
{
//    qDebug() << " event type: " << event->type();
    if(event->type() == QEvent::KeyPress)
    {
        handleKeyPressed((QKeyEvent *)event);
        return true;
    }else if(event->type() == QEvent::GraphicsSceneMouseRelease){
        handleMouseClick((QMouseEvent *)event);
    }else if(event->type() == QEvent::GraphicsSceneWheel){
        handleMouseWheel((QGraphicsSceneWheelEvent *)event);
    }else if(event->type() == QEvent::GraphicsSceneMouseMove){
        handleMouseMove((QGraphicsSceneMouseEvent*)event);
    }else{
        return QObject::eventFilter(object,event);
    }
}

void GameController::LoadPolys()
{
    poly.state = POLY4D_STATE_ACTIVE;
//    poly.attr = POLY4D_ATTR_SHADE_MODE_CONSTANT;
    poly.attr = POLY4D_ATTR_SHADE_MODE_GOURAUD;
//    poly.attr = POLY4D_ATTR_SHADE_MODE_TEXTURE;
    //poly.lit_color[0] =
    poly.vert_color[0] = RGBA32BIT(0, 255, 0, 255);
    poly.vert_color[1] = RGBA32BIT(255, 0, 0, 255);
    poly.vert_color[2] = RGBA32BIT(0, 0, 255, 255);

    poly.verts[0].x = -80;
    poly.verts[0].y = 40;
    poly.verts[0].z = 30;
    poly.verts[0].w = 1;
    poly.verts[0].u0 = 0.3;
    poly.verts[0].v0 = 0.5;


    poly.verts[1].x = 10;
    poly.verts[1].y = 0;
    poly.verts[1].z = 100;
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
//    poly1.attr = POLY4D_ATTR_SHADE_MODE_CONSTANT;
//    poly1.attr = POLY4D_ATTR_SHADE_MODE_TEXTURE;//
    poly1.attr = POLY4D_ATTR_SHADE_MODE_GOURAUD;
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

    Load_Bitmap_File_Qt(&bitmap_file, ":/texture/resource/cloud03.bmp");
//    Load_Bitmap_File(&bitmap_file, "cloud03.bmp");
    poly1.texture = (Bitmap_Iamge_PTR)malloc(sizeof(Bitmap_Image));
    Create_Bitmap(poly1.texture, 0, 0, bitmap_file.bitmapinfoheader.biWidth, bitmap_file.bitmapinfoheader.biHeight, bitmap_file.bitmapinfoheader.biBitCount);
    Load_Image_Bitmap(poly1.texture, &bitmap_file, 0, 0, BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap_file);


    Load_Bitmap_File_Qt(&bitmap_file1, ":/texture/resource/nebgreen03.bmp");
//    Load_Bitmap_File(&bitmap_file1, "cloud03.bmp");
    poly.texture = (Bitmap_Iamge_PTR)malloc(sizeof(Bitmap_Image));
    Create_Bitmap(poly.texture, 0, 0, bitmap_file1.bitmapinfoheader.biWidth, bitmap_file1.bitmapinfoheader.biHeight, bitmap_file1.bitmapinfoheader.biBitCount);
    Load_Image_Bitmap(poly.texture, &bitmap_file1, 0, 0, BITMAP_EXTRACT_MODE_ABS);
    Unload_Bitmap_File(&bitmap_file1);

    Insert_Poly4D_RenderList4D(&render_list, &poly);
    Insert_Poly4D_RenderList4D(&render_list, &poly1);
}

void GameController::LoadModel()
{
    Assimp::Importer importer;
    QFile fi(":/model/resource/Diablo.fbx");
    const aiScene* scene = NULL;
    if(fi.open(QIODevice::ReadOnly))
    {
        scene = importer.ReadFileFromMemory(fi.readAll().data(), fi.size(),aiProcessPreset_TargetRealtime_Quality);
    }
    if(!scene)
    {
        qDebug() << "model load failed: " << importer.GetErrorString();
        return;
    }

    Recursive_Render(scene,scene->mRootNode,5);
}

void GameController::Recursive_Render(const aiScene *scene, const aiNode *node, float scale)
{
//    aiMatrix4x4 m = node->mTransformation;

//    aiMatrix4x4 m2;
//    aiMatrix4x4::Scaling(aiVector3D(scale,scale,scale),m2);
//    m = m*m2;

//    m.Transpose();
//    qDebug() << "mesh num: " << node->mNumMeshes;

    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {

        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        qDebug() << "face num: " << mesh->mNumFaces;

        for(unsigned int j = 0; j < mesh->mNumFaces; j++)
        {

            aiFace* face = &mesh->mFaces[j];
            if(face->mNumIndices != 3)
            {
                qDebug() << "polygon type is not triangle!";
                continue;
            }
            Poly4D poly;
            poly.state = POLY4D_STATE_ACTIVE;
            poly.attr = POLY4D_ATTR_SHADE_MODE_GOURAUD;

            for(unsigned int k = 0; k < face->mNumIndices; k++)
            {
                int vIdx = face->mIndices[k];
                poly.verts[k].x = mesh->mVertices[vIdx].x*scale;
                poly.verts[k].y = mesh->mVertices[vIdx].y*scale;
                poly.verts[k].z = mesh->mVertices[vIdx].z*scale;
                if(mesh->HasNormals())
                {
                    poly.verts[k].nx = mesh->mNormals[vIdx].x;
                    poly.verts[k].ny = mesh->mNormals[vIdx].y;
                    poly.verts[k].nz = mesh->mNormals[vIdx].z;
                    poly.verts[k].nw = 1;
                }


                if(mesh->mColors[0] != NULL)
                {

                    aiColor4D* color = &mesh->mColors[0][vIdx];
                    qDebug() << "color: " << color;
                    poly.vert_color[k] = RGBA32BIT(color->r, color->g, color->b, 255);
                }else{
                    poly.vert_color[k] = RGBA32BIT(255* k/face->mNumIndices, 255, 0, 255);
                }

            }
            Insert_Poly4D_RenderList4D(&render_list,&poly);
        }



    }

    for(unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        Recursive_Render(scene,node->mChildren[i],scale);
    }
}

void GameController::Game_Init()
{
    DDraw_Init(WINDOW_WIDTH,WINDOW_HEIGHT/*,WINDOW_BPP,WINDOW_APP*/);

    Point4D  cam_pos = { 0, 0, -200, 1 };
    Point4D  cam_pos1 = { 0, 0, -300, 1 };

    //这里使用的是弧度值
    Vector4D cam_dir = { 0, 0, 0, 1 };
    Vector4D cam_dir1 = { 0, PI/2, 0, 1 };

    Init_Camera4D(&cam, CAM_MODEL_EULER, &cam_pos, &cam_dir,NULL, 50.0f, 500.0f, 120.0, WINDOW_WIDTH, WINDOW_HEIGHT);
    Init_Camera4D(&cam1, CAM_MODEL_EULER, &cam_pos1,&cam_dir1 ,NULL, 50.0f, 500.0f, 120.0, WINDOW_WIDTH, WINDOW_HEIGHT);

    curr_cam = &cam;

    white.rgba = _RGBA32BIT(50,50,50,0);
    yellow.rgba = _RGBA32BIT(100, 100, 0, 0);

    Init_Lights_Light(AMBIENT_LIGHT_INDEX, LIGHT_STATE_ON, LIGHT_ATTR_AMBIENT, white, white, white, NULL, NULL, 0, 0, 0, 0, 0, 0);

    Vector4D dlight_dir = { 0, 0, PI/2, 0 };
    Init_Lights_Light(INFINITE_LIGHT_INDEX, LIGHT_STATE_ON, LIGHT_ATTR_INFINITE, yellow, yellow, white, NULL, &dlight_dir, 0, 0, 0, 0, 0, 0);

    Create_ZBuffer(&zbuffer, WINDOW_WIDTH, WINDOW_HEIGHT, ZBUFFER_ATTR_32BIT);


    return;
}

void GameController::Game_Main()
{
    static Matrix4X4 mrot;		//通用的旋转矩阵


    //清理绘制的面
    //DDraw_Fill_Surface(lpddsback, 0);
    Clear_Back_Buffer(back_buffer,0);

    // read keyboard and other devices here
    //DInput_Read_Keyboard();

    //初始化渲染列表
    Reset_RenderList4D(&render_list);
//    LoadModel();

    Build_XYZ_Rotation_Matrix4X4(0, ang_y,0 , &mrot);

    //if ((poly1.verts[0].u0 += 0.005) > 1.0) poly1.verts[0].u0 = 0.0;

    //旋转本地坐标
    Transform_RenderList4D(&render_list, &mrot, TRANSFORM_LOCAL_ONLY);

    //模型位置变化到世界坐标
    Model_To_World_RenderList4D(&render_list, &poly_pos);

    //背面剔除
    if(render_state & RENDER_STATE_REMOVE_BACK)
    {
        Remove_Backfaces_RenderList4D(&render_list, curr_cam);
    }

    //光照
    if(render_state & RENDER_STATE_LIGHT)
    {
        Light_RenderList4D_World(&render_list, curr_cam, lights, 2);
    }


    //构建世界到相机的矩阵
    Build_Camera4D_Matrix_Euler(curr_cam, CAM_ROT_SEQ_ZYX);

    //世界坐标到相机坐标的变换
    Transform_RenderList4D(&render_list, &curr_cam->mcam, TRANSFORM_TRANS_ONLY);

    //在相机空间进行裁剪
    Clip_Poly_RenderList4D(&render_list, curr_cam, CLIP_POLY_Z_PLANE);

    //相机到透视
    Camera_To_Perspective_RenderList4D(&render_list, curr_cam);

    //透视到屏幕
    Perspective_To_Screen_RenderList4D(&render_list, curr_cam);

    //DDraw_Lock_Back_Surface();

    //先对zbuffer进行清理
    Clear_ZBuffer(&zbuffer, 1);

//    qDebug() << "draw start";
    //绘制线框
    if(render_state & RENDER_STATE_FRAME)
    {
        Draw_RenderList4D_Wire(&render_list, back_buffer, back_lpitch);
    }

    if(render_state & RENDER_STATE_SOLID)
    {
//        Draw_RenderList4D_Solid(&render_list, back_buffer, back_lpitch,0,0);
        Draw_RenderList4D_Solid(&render_list, back_buffer, back_lpitch, zbuffer.zbuffer, WINDOW_WIDTH * 4);
    }

//

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


    //DDraw_Unlock_Back_Surface();

    //DDraw_Flip();

    //Wait_Clock(30);

    QImage image(WINDOW_WIDTH,WINDOW_HEIGHT,QImage::Format_ARGB32);
//    image.fill(0x110203ff);
//    memcpy(image.bits(), render_list.poly_ptrs[0]->texture->buffer,sizeof(UCHAR) * WINDOW_WIDTH * WINDOW_HEIGHT * 4);
//    free(zbuffer.zbuffer);
//    zbuffer.zbuffer = render_list.poly_ptrs[0]->texture->buffer;
    if(render_state & RENDER_STATE_ZBUFFER)
    {
        memcpy(image.bits(), zbuffer.zbuffer,sizeof(UCHAR) * WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    }else{
        memcpy(image.bits(), back_buffer,sizeof(UCHAR) * WINDOW_WIDTH * WINDOW_HEIGHT * 4);
    }

    if(mytime.elapsed() - curr_time > 1000)
    {
        curr_time = mytime.elapsed();
        str_info.clear();
        str_info.append("fps: ");
        str_info.append(std::to_string(curr_frame).c_str());
        render_info->setText(str_info);
        curr_frame = 0;
    }else{
        curr_frame++;
    }

    QPixmap pm = QPixmap::fromImage(image);
    pixmap->setPixmap(pm);


    return;
}


