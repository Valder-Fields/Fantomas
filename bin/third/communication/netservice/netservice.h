/********************************************************************************************************
*                                       netservice                                                      *
*																										*
*                               (c) Copyright 2019,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   netservice.h                                                                        *
*	Description	:	网络服务器接口函数声明                                                                  *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/
#ifndef __NETSERVICE_H__
#define __NETSERVICE_H__

#include "netservice/netservice_interface.h"

class NETSERVICE_API NetService : public BaseNetService
{
public:
    NetService();
    ~NetService();

    virtual int Start(int protoType = COMM_PROTOCOL_SimpleTlv);

    virtual int Stop();

    NETSRV_SRVDATA * GetServerData();
    NETSRV_CLIDATA * GetClientData(int cliId);
    CommUdpSocket  * GetUdpSocketHandler();

protected:
    //对外接口:模块消息(type:类型 length:消息数据长度 value:消息数据指针)
    virtual int     MessageFrame(int cliId, int commId, int type, int length, unsigned char *value) = 0;

    //对外接口:处理数据帧(type:类型 length:整帧数据长度 value:整帧数据指针)
    virtual int     DealOriginFrame(int cliId, int commId, int type, int frameLen, unsigned char *frame);

    //对外接口:用户线程
    virtual void    UserProc() = 0;

    //对外接口:服务器发送文件
    virtual int     SendFile(int destId, int srcId, char *fullFilePath);
};

#endif // NETSERVICE_H
