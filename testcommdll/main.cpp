#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    /*QFile f("1.txt");
    f.open(QFile::ReadOnly | QFile::WriteOnly);
    char buf[1024] = {0};
    int n =  f.read(buf, 1024);
    int num = 1000;
    if(n)
    {
        num = *(int*)(&buf[0]);
        qDebug() << "app id:" << num;
        w.setWindowTitle(w.windowTitle()+ ":" + QString().setNum(num));
        num += 1000;
    }

    {
        f.seek(0);
        f.write((const char*)(&num), sizeof(int));
    }

    f.close();*/


    return a.exec();
}
