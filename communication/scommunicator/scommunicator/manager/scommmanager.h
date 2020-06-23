/********************************************************************************************************
*                                       scommmanager                                                     *
*																										*
*                                  (c) Copyright 2020                                                   *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   scommmanager.h                                                                       *
*	Description	:	通讯管理-管理                                                                         *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __SCOMMMANAGER_H__
#define __SCOMMMANAGER_H__

#include "../common/scommunicator_def.h"

class CommProtocol;
class CommUdpSocket;

//Qt支持
//#include <QObject>
//class SCOMMUNICATOR_API SCommManager : public QObject
class SCOMMUNICATOR_API SCommManager
{

public:
    SCommManager();
    ~SCommManager();

    int         Init(int protoType);

    int         Quit();

    //对外接口:模块消息(type:类型 length:业务数据长度 value:业务数据指针)
    virtual int     MessageFrame(int cliId, int commId, int type, int length, unsigned char *value) = 0;

    //对外接口:发送数据帧(type:类型 length:业务数据长度 value:业务数据指针)
    virtual int     SendFrame(int cliId, int commId, int type, int length, unsigned char *value);

    //
    virtual void    DispatchProc() = 0;

    //
    virtual void    ServiceProc() = 0;

    //
    virtual void    UserProc() = 0;

public:
    //流程
    //
    void        AcceptProc();

    //
    void        ReadProc();

    //
    void        ForwardingProc();

protected:
    //内存操作
    NETSRV_SRVDATA * GetServerData();
    NETSRV_CLIDATA * GetClientData(int cliId);
    CommUdpSocket  * GetUdpSocketHandler();

    //client内存操作
    COMM_CHANNEL_RXBUF *GetRxBufPtr(int cliId, int commId);
    COMM_CHANNEL_TXBUF *GetTxBufPtr(int cliId, int commId);

    void        GetRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char *val);
    void        PutRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char val);
    void        PutTxbufVal(COMM_CHANNEL_TXBUF *txbuf, unsigned char val);

    int         GetRxbuf(int cliId, int commId, unsigned char *buffer, int bufferLen);
    int         PutRxbuf(int cliId, int commId, unsigned char *buffer, int bufferLen);
    int         PutTxBuf(int cliId, int commId, unsigned char *buffer, int bufferLen);

    //server内存操作
    SRV_COMM_CHANNEL_TXBUF *GetSrvTxBufPtr(int cliId);             //根据CliId获取对应的srvtxbuf
    SRV_COMM_CHANNEL_TXBUF *GetSrvTxBufPtrByRoomId(int roomId);    //根据roomId获取对应的srvtxbuf

    void        PutSrvTxbufVal(SRV_COMM_CHANNEL_TXBUF *txbuf, unsigned char val);

    int         PutSrvForwardingBuf(int cliId, unsigned char *buffer, int frameLen); //转发给所有客户端
    int         PutSrvForwardingBufByRoomId(int roomId, unsigned char *buffer, int frameLen); //转发给房间内所有客户端

protected:
    //protocol
    int         GetAWholeFrame(int cliId, int commId, int &type, unsigned char *frame, int &frameLen);

    int         GetValueByFrame(unsigned char *frame, int frameLen, unsigned char **value, int &valueLen);

    //制作完整帧
    int         MakeAWholeFrame(int type, unsigned char *value, int valueLen, unsigned char *frame, int &frameLen);

public:
    //业务
    //heartbeat
    int         isHeartSendTimeout(int cliId, int commId);
    int         isHeartRecvTimeout(int cliId, int commId);

    int         SetCommConnFlag(int cliId, int commId, int flag);
    int         isConnected(int cliId, int commId);

    void        ClearHeartRecvWaitCount(int cliId, int commId);

    //通道断开,数据复归
    void        CommBrokenProc(int cliId, int commId);

    //关闭所有通道
    void        CloseAllCliSocket();

private:
    CommProtocol *m_protocol;
};


UINT    srv_acceptproc(LPVOID pParam);

UINT    srv_readproc(LPVOID pParam);

UINT    srv_forwardingproc(LPVOID pParam);

UINT    srv_dispatchproc(LPVOID pParam);

UINT    srv_serviceproc(LPVOID pParam);

UINT    srv_userproc(LPVOID pParam);

#endif //__SCOMMMANAGER_H__
