#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QGraphicsView>
#include <QGraphicsScene>

#include <QMainWindow>

#include "gamecontroller.h"

//class QGraphicScene;
//class QGraphicsView;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    Ui::MainWindow *ui;

private slots:
    void adjustViewSize();

private:
    QGraphicsScene* scene;
    QGraphicsView* view;
    GameController* game;

};

#endif // MAINWINDOW_H
