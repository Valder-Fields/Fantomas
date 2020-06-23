

#include <QDebug>
#include <QTime>

#include "fff.h"


AAA::AAA()
{

}

AAA::~AAA()
{

}

int AAA::DealFrame(int type, int length, unsigned char *value)
{
    //测试接收
    switch (type) {
    case 60000:
    {
        TestFrame v;
        memcpy(&v, value, length);

        //(TestFrame *)value;

        //user do sth.
        
    }
        break;

    }

    return TRUE;
}

int AAA::MessageFrame(int type, int length, unsigned char *value)
{
    //测试接收
    switch (type) {
    case COMM_FRAMETYPE_MessageCode:
    {
        MessageCode v;
        memcpy(&v, value, length);

        switch (v.code) {
        case COMM_Code::ConnectSuccess:
        {
            qDebug()<<"ConnectSuccess!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::ConnectFail:
        {
            qDebug()<<"ConnectFail!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::ConnectBreak:
        {
            qDebug()<<"ConnectBreak!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::LoginSuccess:
        {
            qDebug()<<"LoginSuccess!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::LoginFail:
        {
            qDebug()<<"LoginFail!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::CMD_FileSendInit:
        {
            qDebug()<<"CMD_FileSendInit!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::CMD_FileSendFinish:
        {
            qDebug()<<"CMD_FileSendFinish!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::CMD_FileSendError:
        {
            qDebug()<<"CMD_FileSendError!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::CMD_FileRecvFinish:
        {
            qDebug()<<"CMD_FileRecvFinish!"<<QTime::currentTime();
        }
            break;

        case COMM_Code::CMD_FileRecvError:
        {
            qDebug()<<"CMD_FileRecvError!"<<QTime::currentTime();
        }
            break;

        default:
            break;
        }
    }
        break;

    }

    return TRUE;
}
