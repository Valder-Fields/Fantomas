/********************************************************************************************************
*                                       communicator                                                    *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   communicator.h                                                                      *
*	Description	:	通讯管理对外封装类                                                                     *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __fff_H__
#define __fff_H__

#include "communication/communicator/communicator.h"

struct TestFrame
{
    char    item[128];  //评分项目
    float   score;                    //所扣分数
    char    desc[1024]; //各个扣分点的拼接字符串
};

class AAA : public Communicator
{
public:
    AAA();
    ~AAA();

    //对外接口:处理数据帧(type:类型 length:业务数据长度 value:业务数据指针 返回值:成功-TRUE/失败-错误码)
    virtual int     DealFrame(int type, int length, unsigned char *value);

    virtual int     MessageFrame(int type, int length, unsigned char *value);
};


#endif //__COMMUNICATOR_H__
