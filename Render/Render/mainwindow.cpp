#include <QtDebug>
#include <QPainter>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gamecontroller.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), scene(new QGraphicsScene(this)), view(new QGraphicsView(scene,this)),game(new GameController(*scene,this)),
    ui(new Ui::MainWindow)
{
    //ui->setupUi(this);
    setCentralWidget(view);
    resize(WINDOW_WIDTH,WINDOW_HEIGHT);

    qDebug() << "log";

    scene->setSceneRect(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
    QTimer::singleShot(0,this,SLOT(adjustViewSize()));

    QPixmap bg(TILE_SIZE, TILE_SIZE);
    QPainter p(&bg);
    p.setBrush(QBrush(Qt::gray));
    p.drawRect(0, 0, TILE_SIZE, TILE_SIZE);

    view->setBackgroundBrush(QBrush(bg));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::adjustViewSize()
{
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatioByExpanding);
}
