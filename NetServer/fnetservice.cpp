

#include <QDebug>
#include <QTime>

#include "../utils/utils.h"
#include "fnetservice.h"


FNetService::FNetService()
{

}

FNetService::~FNetService()
{

}

int FNetService::MessageFrame(int cliId, int commId, int type, int length, unsigned char *value)
{
    (void)cliId;
    (void)commId;

    //qDebug()<<"cliId:"<<cliId<<"commId:"<<commId<<"type:"<<type;
    switch (type) {
    case COMM_FRAMETYPE_MessageCode:
    {
        MessageCode msg;
        memcpy(&msg, value, length);
        Q_EMIT MessageCodeSignal(msg);
    }
        break;

    case COMM_FRAMETYPE_CommonMessageCode:
    {
        CommonMessageCode msg;
        memcpy(&msg, value, length);
        Q_EMIT CommonMessageCodeSignal(msg);
    }
        break;

    default:
        break;
    }

    return TRUE;
}

int FNetService::DealOriginFrame(int cliId, int commId, int type, int frameLen, unsigned char *frame)
{
    (void)cliId;
    (void)commId;
    (void)type;
    (void)frameLen;
    (void)frame;

    /*UtilFunc::PrintHexStr((char *)frame, frameLen);
    //解析
    int valueLen = 0;
    unsigned char *value = NULL;
    int ret = GetValueByFrame(frame, frameLen, &value, valueLen);

    switch (type) {
    case 41001:
    {
        CoordinateData v;
        memcpy(&v, (char *)value, 24);

        memcpy(&v.id, value, 4);
        memcpy(&v.x, value+4, 8);
        memcpy(&v.y, value+12, 8);

        v.x = ((CoordinateData *)value)->x;
        v.y = ((CoordinateData *)value)->y;

        CoordinateDataGis gis;
        gis.x = v.x;
        gis.y = v.y;

        char src[17];
        memset(src, 0, 256);
        memcpy(src, (value+4), sizeof(CoordinateDataGis));

        char dest[17];
        char dest1[256];
        memset(dest, 0, 256);
        memcpy(dest, (char *)&gis, sizeof(CoordinateDataGis));

        int a1 = sizeof(CoordinateData);
        int b1 = sizeof(CoordinateDataGis);

        //int ret = GetUdpSocketHandler()->Write((char *)&gis, sizeof(CoordinateDataGis));
        int ret = GetUdpSocketHandler()->Write((char *)(value+4), sizeof(CoordinateDataGis));
        qDebug()<<"udp send:"<<ret<<"x:"<<gis.x<<"y:"<<gis.y<<QTime::currentTime();

    }
        break;

    default:
        break;
    }


    //DealXXX(cliId, commId, (FRM_DownFile *)value);*/

    return FALSE;
}

void FNetService::UserProc()
{

}

NETSRV_SRVDATA * FNetService::GetServerData()
{
    return NetService::GetServerData();
}

NETSRV_CLIDATA * FNetService::GetClientData(int cliId)
{
    return NetService::GetClientData(cliId);
}

CommUdpSocket * FNetService::GetUdpSocketHandler()
{
    return NetService::GetUdpSocketHandler();
}

int FNetService::InitCommUdpSocket(UdpSocketCfg &cfg)
{
    int ret = GetUdpSocketHandler()->Init(cfg);
    return ret;
}
