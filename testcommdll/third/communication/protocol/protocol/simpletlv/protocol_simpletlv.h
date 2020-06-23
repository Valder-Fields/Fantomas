/********************************************************************************************************
*                                       PROTOCOL LIB                                                    *
*                                                                                                       *
*                                   (c) Copyright 2020                                                  *
*                                   All Rights Reserved                                                 *
*                                                                                                       *
*   FileName    :   protocol_simpletlv.h                                                                *
*   Description :   通讯协议:简单TLV协议                                                                *
*   Author      :   Valder-Fields                                                                                 *
*   Date        :                                                                                       *
*   Remark      :   TLV:2字节Type+4字节Length+Value                                                     *
*                                                                                                       *
*   No.         Date         Modifier        Description                                                *
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#ifndef __PROTOCOL_SIMPLETLV_H__
#define __PROTOCOL_SIMPLETLV_H__

#include <string.h>

#include "../base/baseprotocol.h"


#define     SIMPLETLV_Frame_Type              2               //帧头
#define     SIMPLETLV_Frame_Length            4               //帧长度

#define     SIMPLETLV_WARNING_LastLenLessThanTL     -100      //警告:缓存剩余字节<T+L
#define     SIMPLETLV_WARNING_LastLenLessThanTLV    -101      //警告:缓存剩余字节<T+L+V
#define     SIMPLETLV_WARNING_HeadEqualTail         -102      //警告:缓存头尾指针相等
#define     SIMPLETLV_WARNING_FrameMoreThanMaxLen   -200      //警告:帧长度大于最大允许帧长

#define     SIMPLETLV_WARNING_SendFrameLenOverLimit -300      //发送帧长越限

class PROTOCOL_API ProtoSimpleTlv : public CommProtocol
{
public:
    ProtoSimpleTlv();
    ~ProtoSimpleTlv();

    virtual int     GetAWholeFrame(COMM_CHANNEL_RXBUF *rxbuf, int &type, unsigned char *frame, int &frameLen);
    virtual int     MakeAWholeFrame(int type, unsigned char *value, int valueLen, unsigned char *frame, int &frameLen);
    virtual int     GetValueByFrame(unsigned char *frame, int frameLen, unsigned char **value, int &valueLen);

private:
    int             ParseFrameType(COMM_CHANNEL_RXBUF *rxbuf, int typeLen = SIMPLETLV_Frame_Type);
    int             ParseFrameLenth(COMM_CHANNEL_RXBUF *rxbuf, int lenLen = SIMPLETLV_Frame_Length);

    int             GetCommRxbuf(COMM_CHANNEL_RXBUF *rxbuf, unsigned char *buffer, int bufferLen);
};



#endif //__PROTOCOL_INTERFACE_H__
