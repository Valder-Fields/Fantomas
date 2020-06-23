/********************************************************************************************************
*                                       commmanager                                                     *
*																										*
*                                  (c) Copyright 2020                                                   *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   commmanager.h                                                                       *
*	Description	:	通讯管理-管理                                                                         *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __COMMMANAGER_H__
#define __COMMMANAGER_H__

#include "../common/communicator_def.h"

class CommProtocol;

class COMMUNICATOR_API CommManager
{

public:
    CommManager();
    ~CommManager();

    int         Init(int protoType);

    int         Quit();

    //对外接口:获取配置-uuid
    virtual int     GetUuid();

    //对外接口:获取配置-exeId
    virtual int     GetExeId();

    //对外接口:处理数据帧(type:类型 length:业务数据长度 value:业务数据指针)
    virtual int     DealFrame(int type, int length, unsigned char *value) = 0;

    //对外接口:发送数据帧(type:类型 length:业务数据长度 value:业务数据指针)
    virtual int     SendFrame(int type, int length, unsigned char *value);

    //对外接口:模块消息(type:类型 length:消息数据长度 value:消息数据指针)
    virtual int     MessageFrame(int type, int length, unsigned char *value) = 0;

    //对外接口:文件发送
    virtual int     SendFile(int destId, int srcId, char *fullFilePath);

public:
    //
    void        ReadProc();

    //
    void        ForwardingProc();

    //
    void        DispatchProc();

protected:
    //内存操作
    COMM_CHANNEL_RXBUF *GetRxBufPtr();
    COMM_CHANNEL_TXBUF *GetTxBufPtr();

    int         GetRxbuf(unsigned char *buffer, int bufferLen);
    int         PutRxbuf(unsigned char *buffer, int bufferLen);
    int         PutTxBuf(unsigned char *buffer, int bufferLen);

    void        GetRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char *val);
    void        PutRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char val);
    void        PutCliTxbufVal(COMM_CHANNEL_TXBUF *txbuf, unsigned char val);

    //protocol
    int         GetAWholeFrame(int &type, unsigned char *frame, int &frameLen);

    int         GetValueByFrame(unsigned char *frame, int frameLen, unsigned char **value, int &valueLen);

private:
    //业务
    //调度-接收
    int         RxChannelProc();

    //调度-发送
    int         TxChannelProc();

    //连接状态检查
    void        CheckConnectProc();

    //heartbeat
    //
    void        HeartbeatProc();

    int         isHeartSendTimeout();
    int         isHeartRecvTimeout();

    //通道操作
    int         SetCommConnFlag(int flag);
    int         isConnected();

    //通道断开,数据复归
    void        CommBrokenProc();

    //命令处理
    void        CommandProc();

    //文件发送
    int         FileSendProc();
    void        InitFileSendProc();
    int         CheckFileSendProcState(CMD_FILESEND *fileSend, int state);
    void        StopFileSendProc();
    int         SendFileSendState(int state);

    //模块内部接收业务处理
    int         DealOriginFrame(int type, int frameLen, unsigned char *frame);

    //文件接收
    int         FileRecvProc(int type, int frameLen, unsigned char *frame);
    int         CheckFileRecvProcState(CMD_FILERECV *fileRecv, int state);
    void        InitFileRecvProc(CMD_FILERECV *fileRecv);
    void        StopFileRecvProc();
    int         SendFileRecvState(int state);

public:
    //地址解析
    void        ParseAddressId(int addrId, int &uuid, int &exeId);
    void        MakeAddressId(int uuid, int exeId, int &addrId);

private:
    CommProtocol *m_protocol;
};


UINT    srv_readproc(LPVOID pParam);

UINT    srv_forwardingproc(LPVOID pParam);

UINT    srv_dispatchproc(LPVOID pParam);

#endif //__COMMMANAGER_H__
