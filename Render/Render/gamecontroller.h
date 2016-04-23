#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QTime>

#include "poly.h"
#include "math3d.h"
#include "draw.h"
#include "camera.h"
#include "draw_qt.h"
#include "light.h"
#include "resource.h"

class QGraphicsScene;
class QGraphicsView;
class QGraphicsSimpleTextItem;
class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;
class QGraphicsPixmapItem;
struct aiScene;
struct aiNode;

#define TILE_SIZE 50
#define RENDER_STATE_FRAME          0x1
#define RENDER_STATE_SOLID          0x2
#define RENDER_STATE_FRAME_SOLID    0x3
#define RENDER_STATE_LIGHT          0x4
#define RENDER_STATE_REMOVE_BACK    0x8
#define RENDER_STATE_ZBUFFER        0x10

class GameController : QObject
{
    Q_OBJECT
public:
    GameController(QGraphicsScene& scene, QObject *parent = 0);
    ~GameController();

    void Game_Main();
    void Game_Init();

protected:
    bool eventFilter(QObject *, QEvent *);

private slots:
    void update();

private:
    void handleKeyPressed(QKeyEvent *event);
    void handleMouseClick(QMouseEvent* event);
    void handleMouseWheel(QGraphicsSceneWheelEvent* event);
    void handleMouseMove(QGraphicsSceneMouseEvent* event);
    void LoadModel();
    void LoadPolys();
    void Recursive_Render(const aiScene* scene, const aiNode* node, float scale);

    QTime  mytime;
    QTimer timer;

    QGraphicsScene &scene;

    QGraphicsSimpleTextItem* render_info;
    QGraphicsPixmapItem* pixmap;

    Camera4D cam;
    Camera4D cam1;
    Camera4D_PTR curr_cam;

    RenderList4D render_list;
    Poly4D poly;
    Poly4D poly1;

    RGBA white;
    RGBA yellow;

    Bitmap_File bitmap_file;
    Bitmap_File bitmap_file1;

    Point4D poly_pos = { 0, 0, 0, 1 };


    ZBUFFER zbuffer;

    bool is_update = true;

    float ang_y = 0;		//Ðý×ª½Ç¶È
    float ang_x = 0;
    float ang_z = 0;
    float ang_y_delta = 0.01;

    int render_state = RENDER_STATE_FRAME;
    QString str_info;

    int curr_frame = 0;
    int curr_time = 0;
};

#endif // GAMECONTROLLER_H
