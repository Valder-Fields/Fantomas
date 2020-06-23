
#include <QDateTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<MessageCode>("MessageCode");
    qRegisterMetaType<CommonMessageCode>("CommonMessageCode");
    connect(&service, &FNetService::MessageCodeSignal, this, &MainWindow::MessageCodeSlot);
    connect(&service, &FNetService::CommonMessageCodeSignal, this, &MainWindow::CommonMessageCodeSlot);

    m_runTimer = new QTimer();
    connect(m_runTimer, &QTimer::timeout, this, &MainWindow::OnRunTimer);
    m_runTimer->start(1000);

    service.Start();
}

MainWindow::~MainWindow()
{
    service.Stop();
    delete ui;
}

void MainWindow::SysRunDataCalc()
{
    CalcRecvAllFlow_m();
    CalcRecvUnitFlow_kps();
    CalcRecvPeakFlow_kps();

    CalcRecvAllFrame_f();
    CalcRecvUnitFrame_fps();
    CalcRecvPeakFrame_fps();

    CalcSendAllFlow_m();
    CalcSendUnitFlow_kps();
    CalcSendPeakFlow_kps();

    CalcSendAllFrame_f();
    CalcSendUnitFrame_fps();
    CalcSendPeakFrame_fps();

    CalcForwardingCache1UnitVal();
    CalcForwardingCache1PeakVal();
    CalcForwardingCache1UnitRate();
    CalcForwardingCache1PeakRate();
}

float MainWindow::CalcRecvAllFlow_m()
{
    return (float)service.GetServerData()->runData.recvAllFlow/(float)1024/(float)1024;
}

float MainWindow::CalcRecvUnitFlow_kps()
{
    //
    service.GetServerData()->runData.recvUnitFlow = service.GetServerData()->runData.recvAllFlow - service.GetServerData()->runData.lastRecvAllFlow;

    //
    service.GetServerData()->runData.lastRecvAllFlow = service.GetServerData()->runData.recvAllFlow;

    //
    float unitKps = (float)(service.GetServerData()->runData.recvUnitFlow)/(float)1024;

    return unitKps;
}

float MainWindow::CalcRecvPeakFlow_kps()
{
    //
    //NETSRV_RUNDATA *p = &service.GetServerData()->runData;
    if (service.GetServerData()->runData.recvUnitFlow > service.GetServerData()->runData.recvPeakFlow) {
        service.GetServerData()->runData.recvPeakFlow = service.GetServerData()->runData.recvUnitFlow;
    }

    //
    float peakKps = (float)service.GetServerData()->runData.recvPeakFlow/(float)1024;

    return peakKps;
}

int MainWindow::CalcRecvAllFrame_f()
{
    return service.GetServerData()->runData.recvAllFrame;
}

int MainWindow::CalcRecvUnitFrame_fps()
{
    service.GetServerData()->runData.recvUnitFrame = service.GetServerData()->runData.recvAllFrame - service.GetServerData()->runData.lastRecvAllFrame;

    service.GetServerData()->runData.lastRecvAllFrame = service.GetServerData()->runData.recvAllFrame;

    return service.GetServerData()->runData.recvUnitFrame;
}

int MainWindow::CalcRecvPeakFrame_fps()
{
    if (service.GetServerData()->runData.recvUnitFrame > service.GetServerData()->runData.recvPeakFrame) {
        service.GetServerData()->runData.recvPeakFrame = service.GetServerData()->runData.recvUnitFrame;
    }

    return service.GetServerData()->runData.recvPeakFrame;
}

float MainWindow::CalcSendAllFlow_m()
{
    return (float)service.GetServerData()->runData.sendAllFlow/(float)1024/(float)1024;
}

float MainWindow::CalcSendUnitFlow_kps()
{
    //
    service.GetServerData()->runData.sendUnitFlow = service.GetServerData()->runData.sendAllFlow - service.GetServerData()->runData.lastSendAllFlow;

    //
    service.GetServerData()->runData.lastSendAllFlow = service.GetServerData()->runData.sendAllFlow;

    //
    float unitKps = (float)(service.GetServerData()->runData.sendUnitFlow)/(float)1024;

    return unitKps;
}

float MainWindow::CalcSendPeakFlow_kps()
{
    //
    if (service.GetServerData()->runData.sendUnitFlow > service.GetServerData()->runData.sendPeakFlow) {
        service.GetServerData()->runData.sendPeakFlow = service.GetServerData()->runData.sendUnitFlow;
    }

    //
    float peakKps = (float)service.GetServerData()->runData.sendPeakFlow/(float)1024;

    return peakKps;
}

int MainWindow::CalcSendAllFrame_f()
{
    return service.GetServerData()->runData.sendAllFrame;
}

int MainWindow::CalcSendUnitFrame_fps()
{
    service.GetServerData()->runData.sendUnitFrame = service.GetServerData()->runData.sendAllFrame - service.GetServerData()->runData.lastSendAllFrame;

    service.GetServerData()->runData.lastSendAllFrame = service.GetServerData()->runData.sendAllFrame;

    return service.GetServerData()->runData.sendUnitFrame;
}

int MainWindow::CalcSendPeakFrame_fps()
{
    if (service.GetServerData()->runData.sendUnitFrame > service.GetServerData()->runData.sendPeakFrame) {
        service.GetServerData()->runData.sendPeakFrame = service.GetServerData()->runData.sendUnitFrame;
    }

    return service.GetServerData()->runData.sendPeakFrame;
}

int MainWindow::CalcForwardingCache1UnitVal()
{
    //
    service.GetServerData()->runData.forwardingCache1UnitVal = (service.GetServerData()->roomData[0].commData.txbuf.tail+COMM_SRVTXBUF_LEN-service.GetServerData()->roomData[0].commData.txbuf.head)%COMM_SRVTXBUF_LEN;

    return service.GetServerData()->runData.forwardingCache1UnitVal;
}

int MainWindow::CalcForwardingCache1PeakVal()
{
    //
    if (service.GetServerData()->runData.forwardingCache1UnitVal > service.GetServerData()->runData.forwardingCache1PeakVal) {
        service.GetServerData()->runData.forwardingCache1PeakVal = service.GetServerData()->runData.forwardingCache1UnitVal;
    }

    return service.GetServerData()->runData.forwardingCache1PeakVal;
}

float MainWindow::CalcForwardingCache1UnitRate()
{
    //
    service.GetServerData()->runData.forwardingCache1UnitRate = (float)service.GetServerData()->runData.forwardingCache1UnitVal/(float)COMM_SRVTXBUF_LEN;

    return service.GetServerData()->runData.forwardingCache1UnitRate;
}

float MainWindow::CalcForwardingCache1PeakRate()
{
    if (service.GetServerData()->runData.forwardingCache1UnitRate > service.GetServerData()->runData.forwardingCache1PeakRate) {
        service.GetServerData()->runData.forwardingCache1PeakRate = service.GetServerData()->runData.forwardingCache1UnitRate;
    }

    return service.GetServerData()->runData.forwardingCache1PeakRate;
}

void MainWindow::OnRunTimer()
{

    //
    SysRunDataCalc();

    char tmpText[256];

    //接收瞬时流量:
    sprintf(tmpText, "%.2f", (float)service.GetServerData()->runData.recvUnitFlow/(float)1024);
    ui->lineEdit_RecvUnitFlow->setText(tmpText);

    //接收峰值流量:
    sprintf(tmpText, "%.2f", (float)service.GetServerData()->runData.recvPeakFlow/(float)1024);
    ui->lineEdit_RecvPeakFlow->setText(tmpText);

    //接收总流量:
    sprintf(tmpText, "%.2f", (float)service.GetServerData()->runData.recvAllFlow/(float)1024/(float)1024);
    ui->lineEdit_RecvAllFlow->setText(tmpText);

    //接收瞬时帧数:
    sprintf(tmpText, "%d", service.GetServerData()->runData.recvUnitFrame);
    ui->lineEdit_RecvUnitFrame->setText(tmpText);

    //接收峰值帧数:
    sprintf(tmpText, "%d", service.GetServerData()->runData.recvPeakFrame);
    ui->lineEdit_RecvPeakFrame->setText(tmpText);

    //接收总帧数:
    sprintf(tmpText, "%d", service.GetServerData()->runData.recvAllFrame);
    ui->lineEdit_RecvAllFrame->setText(tmpText);

    //发送瞬时流量:
    sprintf(tmpText, "%.2f", (float)service.GetServerData()->runData.sendUnitFlow/(float)1024);
    ui->lineEdit_SendUnitFlow->setText(tmpText);

    //发送峰值流量:
    sprintf(tmpText, "%.2f", (float)service.GetServerData()->runData.sendPeakFlow/(float)1024);
    ui->lineEdit_SendPeakFlow->setText(tmpText);

    //发送总流量:
    sprintf(tmpText, "%.2f", (float)service.GetServerData()->runData.sendAllFlow/(float)1024/(float)1024);
    ui->lineEdit_SendAllFlow->setText(tmpText);

    //发送瞬时帧数:
    sprintf(tmpText, "%d", service.GetServerData()->runData.sendUnitFrame);
    ui->lineEdit_SendUnitFrame->setText(tmpText);

    //发送峰值帧数:
    sprintf(tmpText, "%d", service.GetServerData()->runData.sendPeakFrame);
    ui->lineEdit_SendPeakFrame->setText(tmpText);

    //发送总帧数:
    sprintf(tmpText, "%d", service.GetServerData()->runData.sendAllFrame);
    ui->lineEdit_SendAllFrame->setText(tmpText);

    //系统
    //转发缓存瞬时使用量:
    sprintf(tmpText, "%d", service.GetServerData()->runData.forwardingCache1UnitVal);
    ui->lineEdit_ForwardingCacheUnitVal->setText(tmpText);

    //转发缓存峰值使用量:
    sprintf(tmpText, "%d", service.GetServerData()->runData.forwardingCache1PeakVal);
    ui->lineEdit_ForwardingCachePeakVal->setText(tmpText);

    //转发缓存瞬时利用率:
    sprintf(tmpText, "%.2f", service.GetServerData()->runData.forwardingCache1UnitRate);
    ui->lineEdit_ForwardingCacheUnitRate->setText(tmpText);

    //转发缓存峰值利用率:
    sprintf(tmpText, "%.2f", service.GetServerData()->runData.forwardingCache1PeakRate);
    ui->lineEdit_ForwardingCachePeakRate->setText(tmpText);

    //运行时间
    int curTime = QDateTime::currentDateTime().toTime_t();
    int durTime = curTime-service.GetServerData()->runData.startTime;

    int day = (durTime)/(24*3600);

    int hour = (durTime)%(24*3600)/3600;
    int minute = (durTime)%3600/60;
    int second = (durTime)%60;
    sprintf(tmpText, "%02d天 %02d:%02d:%02d", day, hour, minute, second);
    ui->lineEdit_sysRunTime->setText(QString::fromLocal8Bit(tmpText));

}

void MainWindow::MessageCodeSlot(MessageCode msg)
{
    /*switch (msg.code) {
    case 1:
    {

    }
        break;
    default:
        break;
    }*/

    //ui->textEdit->append(QString::fromLocal8Bit(msg.code));
}

void MainWindow::CommonMessageCodeSlot(CommonMessageCode msg)
{
    /*switch (msg.code) {
    case COMM_Code::ConnectSuccess:
    {

    }
        break;
    default:

        break;
    }*/

    ui->textEdit->append(QString::fromLocal8Bit(msg.content));
}
