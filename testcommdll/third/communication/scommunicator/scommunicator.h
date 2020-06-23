/********************************************************************************************************
*                                       scommunicator                                                    *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   scommunicator.h                                                                       *
*	Description	:	通讯管理对外封装类                                                                     *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __SCOMMUNICATOR_H__
#define __SCOMMUNICATOR_H__

#include "scommunicator/scommunicator_interface.h"

class SCOMMUNICATOR_API SCommunicator : public SCommManager
{
public:
    SCommunicator();
    ~SCommunicator();

    virtual int Start(int protoType = COMM_PROTOCOL_SimpleTlv);

    virtual int Stop();

    //对外接口:模块消息(type:类型 length:消息数据长度 value:消息数据指针)
    virtual int     MessageFrame(int cliId, int commId, int type, int length, unsigned char *value) = 0;

    //对外接口:发送数据帧(type:类型 length:业务数据长度 value:业务数据指针)
    virtual int     SendFrame(int cliId, int commId, int type, int length, unsigned char *value);

    //
    virtual void    DispatchProc() = 0;

    //
    virtual void    ServiceProc() = 0;

    //
    virtual void    UserProc() = 0;

protected:
    //功能接口:制作完整帧
    int     MakeAWholeFrame(int type, unsigned char *value, int valueLen, unsigned char *frame, int &frameLen);

private:
    SCommService *m_service;
};


#endif //__SCOMMUNICATOR_H__
