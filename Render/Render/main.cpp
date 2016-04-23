#include "mainwindow.h"
#include <assimp/Importer.hpp>
#include <QApplication>
#include <QTextStream>
#include <QBitmap>
#include <QFile>

int main(int argc, char *argv[])
{
    QTextStream out(stdout);
    QApplication a(argc, argv);
    MainWindow w;
    //w.repaint();

    //QLabel label("123");
    //QBitmap bitmap = QBitmap.fromData(QSize(4,4),)

    QFile file(QString("../resource/bmp1.bmp"));

    Assimp::Importer importer;// = new Assimp::Importer();
    //importer.ReadFile();

//    QPainter painter;
//    QPixmap pm("resource/bmp1.png");
//    painter.drawPixmap(100,10,100,100,pm);


    QImage image(640,480,QImage::Format_ARGB32);
    //image.bits()


    //out << "File Exit: " + file.exists() << endl;
    w.show();

    //label.show();

    return a.exec();
}
