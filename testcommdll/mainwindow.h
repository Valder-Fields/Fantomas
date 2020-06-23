#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "fff.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Quit();

    template<typename T>
    int         MakeFrameValue(T &v, unsigned char *value);

public Q_SLOTS:
    void    on_pushButton_test_clicked();

private:
    Ui::MainWindow *ui;

    AAA aaaaa;
};

#endif // MAINWINDOW_H
