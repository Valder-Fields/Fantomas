#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <QMainWindow>
#include <QMessageBox>

#include "fnetservice.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public Q_SLOTS:
    void OnRunTimer();

    void MessageCodeSlot(MessageCode msg);

    void CommonMessageCodeSlot(CommonMessageCode msg);

private:
    Ui::MainWindow *ui;

    FNetService service;

    QTimer      *m_runTimer;    //运行刷新

public:
    //系统计算
    void        SysRunDataCalc();

    float       CalcRecvAllFlow_m();
    float       CalcRecvUnitFlow_kps();
    float       CalcRecvPeakFlow_kps();

    int         CalcRecvAllFrame_f();
    int         CalcRecvUnitFrame_fps();
    int         CalcRecvPeakFrame_fps();

    float       CalcSendAllFlow_m();
    float       CalcSendUnitFlow_kps();
    float       CalcSendPeakFlow_kps();

    int         CalcSendAllFrame_f();
    int         CalcSendUnitFrame_fps();
    int         CalcSendPeakFrame_fps();

    int         CalcForwardingCache1UnitVal();
    int         CalcForwardingCache1PeakVal();
    float       CalcForwardingCache1UnitRate();
    float       CalcForwardingCache1PeakRate();
};

#endif // MAINWINDOW_H
