#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //a.setApplicationDisplayName(QString::fromLocal8Bit("联网服务器"));

    MainWindow w;
    w.show();
    //w.showMinimized();

    return a.exec();
}
