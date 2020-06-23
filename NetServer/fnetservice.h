
#ifndef __FNETSERVICE_H__
#define __FNETSERVICE_H__

#include <QtWidgets>
#include "commsocket/commsocket.h"
#include "netservice/netservice.h"


struct CoordinateData
{
    int id;
    double x;
    double y;
};

struct CoordinateDataGis
{
    double x;
    double y;
};

class FNetService : public QObject, public NetService
{
    Q_OBJECT

public:
    FNetService();
    ~FNetService();

    virtual int     MessageFrame(int cliId, int commId, int type, int length, unsigned char *value);

    virtual int     DealOriginFrame(int cliId, int commId, int type, int frameLen, unsigned char *frame);

    virtual void    UserProc();

    NETSRV_SRVDATA * GetServerData();
    NETSRV_CLIDATA * GetClientData(int cliId);
    CommUdpSocket  * GetUdpSocketHandler();
    int             InitCommUdpSocket(UdpSocketCfg &cfg);

Q_SIGNALS:
    void  MessageCodeSignal(MessageCode msg);

    void  CommonMessageCodeSignal(CommonMessageCode msg);
};


#endif //__FNETSERVICE_H__
