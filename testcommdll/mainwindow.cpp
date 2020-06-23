
#include "ui_mainwindow.h"

#include <QDebug>
#include <QTime>
#include <QThread>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug()<<"start begin!"<<QTime::currentTime();
    aaaaa.Start();
    qDebug()<<"start end!"<<QTime::currentTime();
}

MainWindow::~MainWindow()
{
    Quit();

    delete ui;
}

void MainWindow::Quit()
{
    aaaaa.Stop();
}

template<typename T>
int MainWindow::MakeFrameValue(T &v, unsigned char *value)
{
    if (value == NULL) {
        return FALSE;
    }

    memcpy(value,&v, sizeof(T));

    return TRUE;
}

void MainWindow::on_pushButton_test_clicked()
{
    TestFrame v;
    sprintf(v.item, "%s", QString::fromLocal8Bit("你好,测试123abc!").toLocal8Bit().constData());
    v.score = 3.14159f;
    sprintf(v.item, "%s", "测试1234abcd!!!");

    int type = 60000;
    int length = sizeof(TestFrame);

    //测试发送
    aaaaa.SendFrame(type, length, (unsigned char *)&v);
}

