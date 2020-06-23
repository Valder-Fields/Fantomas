/********************************************************************************************************
*                                       netservice                                                      *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   service.h                                                                           *
*	Description	:	网络服务核心模块声明                                                                   *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/
#ifndef __SERVICE_H__
#define __SERVICE_H__

#include "../common/netservice_def.h"
#include "../../../scommunicator/scommunicator.h"

class NETSERVICE_API BaseNetService : public SCommunicator
{
public:
    BaseNetService();
    ~BaseNetService();

    virtual int Start(int protoType = COMM_PROTOCOL_SimpleTlv);

    virtual int Stop();

protected:

    //对外接口:模块消息(type:类型 length:业务数据长度 value:业务数据指针)
    virtual int     MessageFrame(int cliId, int commId, int type, int length, unsigned char *value) = 0;

    //对外接口:处理数据帧(type:类型 length:整帧数据长度 value:整帧数据指针)
    virtual int     DealOriginFrame(int cliId, int commId, int type, int frameLen, unsigned char *frame) = 0;

    //对外接口:服务器发送文件
    virtual int     SendFile(int destId, int srcId, char *fullFilePath);

private:
    //服务器业务
    //调度
    virtual void    DispatchProc();

    //调度-接收
    int             RxChannelProc(int cliId, int commId);

    //调度-发送
    int             TxChannelProc(int cliId, int commId);

    //登录
    int             DealLoginReq(int cliId, int commId, FRM_LoginReq &v);

    //登录-校验
    int             CheckLoginValid(int curCliId, int curCommId, int uuid, int exeId);

    //心跳
    int             DealHeartbeat(int cliId, int commId);

    //服务
    virtual void    ServiceProc();

    //用户线程
    virtual void    UserProc() = 0;

    //服务-保存
    NETSRV_SRVSAVE * GetSrvSaveDataPtrByRoomId(int roomId); //根据roomId获取对应的srvsavedata
    int             SaveProcByRoomId(int roomId);
    int             SaveTrainFileByProcData(char *fileName, int beginInfoIndex, int infoNums, NETSRV_SAVEPROCDATA *procData, SRV_COMM_CHANNEL_TXBUF *txbuf);
    int             SaveTrainFile(const char *fileName, unsigned char *buffer, int bufferLen);

    //服务-回放
    NETSRV_SRVPLAY * GetSrvPlayDataPtrByRoomId(int roomId);
    int             PlayProcByRoomId(int roomId);

    int             PlayBackOnceTrainRecord(int roomId);
    int             ParseOnceTrainRecord(int roomId);
    int             ParseOneTrainRecord(int roomId);

private:
    //外部业务
    //下发想定
    int             DealSendSet(int cliId, int commId, FRM_SendSet *v);

    //训练控制命令处理
    int             DealControlTrain(int cliId, int commId, FRM_ControlTrain *v);

    //训练控制命令回执处理
    int             DealControlTrainRet(int cliId, int commId, FRM_ControlTrainRet *v);

    //回放速率
    int             DealPlaybackRate(int cliId, int commId, GL_FastForward *v);

    //文件接收
    int             FileRecvProc(int cliId, int commId, int type, int frameLen, unsigned char *frame);
    int             CheckFileRecvProcState(CMD_FILERECV *fileRecv, int state);
    void            InitFileRecvProc(CMD_FILERECV *fileRecv);
    void            StopFileRecvProc(int cliId, int commId);
    int             SendFileRecvState(int cliId, int commId, int state);

    //召唤文件目录
    int             DealCallFileDir(int cliId, int commId, FRM_CallFileDir *v);

    //下载文件
    int             DealDownFile(int cliId, int commId, FRM_DownFile *v);

    //命令处理
    void            CommandProc(int cliId, int commId);

    //命令处理-文件发送
    int             FileSendProc(int cliId, int commId);
    void            InitFileSendProc(int cliId, int commId);
    int             CheckFileSendProcState(int cliId, int commId, int state);
    void            StopFileSendProc(int cliId, int commId);
    int             SendFileSendState(int cliId, int commId, int state);

protected:
    //转发规则

    //exe单播
    int             PutTxBufByExeId(int exeId, unsigned char *buffer, int bufferLen);

    //转发(destId=uuid*10000+exeId)
    int             SendByDestId(int destId, unsigned char *buffer, int bufferLen);

public:
    //地址解析
    void            ParseAddressId(int addrId, int &uuid, int &exeId);
    void            MakeAddressId(int uuid, int exeId, int &addrId);

};



#endif // __SERVICE_H__
