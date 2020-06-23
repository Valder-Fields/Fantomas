/********************************************************************************************************
*                                       PROTOCOL LIB                                                    *
*																										*
*                                    (c) Copyright 2020                                                 *
*                                   All Rights Reserved                                                 *
*																										*
*	FileName	:   baseprotocol.h                                                                       *
*	Description	:	通讯协议库-抽象协议头文件                                                               *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __BASEPROTOCOL_H__
#define __BASEPROTOCOL_H__

#include "../common/protocol_def.h"


class PROTOCOL_API CommProtocol
{
public:
    CommProtocol();
    virtual ~CommProtocol();

    virtual int     GetAWholeFrame(COMM_CHANNEL_RXBUF *rxbuf, int &type, unsigned char *frame, int &frameLen) = 0;
    virtual int     MakeAWholeFrame(int type, unsigned char *value, int valueLen, unsigned char *frame, int &frameLen) = 0;
    virtual int     GetValueByFrame(unsigned char *frame, int frameLen, unsigned char **value, int &valueLen) = 0;
};

#endif //__BASEPROTOCOL_H__
