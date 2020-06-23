/********************************************************************************************************
*                                       communicator                                                    *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   communicator.h                                                                       *
*	Description	:	通讯管理对外封装类                                                                     *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __COMMUNICATOR_H__
#define __COMMUNICATOR_H__

#include "communicator/communicator_interface.h"


class COMMUNICATOR_API Communicator : public CommManager
{
public:
    Communicator();
    ~Communicator();

    int Start(int protoType = COMM_PROTOCOL_SimpleTlv);
    int Stop();

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

    //对外接口:发送文件
    virtual int     SendFile(int destId, int srcId, char *fullFilePath);

private:
    CommService *m_service;
};


#endif //__COMMUNICATOR_H__
