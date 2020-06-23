
#include "netservice.h"


NetService::NetService()
{

}

NetService::~NetService()
{

}

int NetService::Start(int protoType)
{
    return BaseNetService::Start(protoType);
}

int NetService::Stop()
{
    return BaseNetService::Stop();
}

NETSRV_SRVDATA * NetService::GetServerData()
{
    return SCommManager::GetServerData();
}

NETSRV_CLIDATA * NetService::GetClientData(int cliId)
{
    return SCommManager::GetClientData(cliId);
}

CommUdpSocket * NetService::GetUdpSocketHandler()
{
    return SCommManager::GetUdpSocketHandler();
}

int NetService::DealOriginFrame(int cliId, int commId, int type, int frameLen, unsigned char *frame)
{
    (void)cliId;
    (void)commId;
    (void)type;
    (void)frameLen;
    (void)frame;

    return FALSE;
}

int NetService::SendFile(int destId, int srcId, char *fullFilePath)
{
    return BaseNetService::SendFile(destId, srcId, fullFilePath);
}
