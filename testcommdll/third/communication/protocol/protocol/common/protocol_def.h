
/********************************************************************************************************
*                                       PROTOCOL LIB                                                    *
*                                                                                                       *
*                               (c) Copyright 2020                                                      *
*                                  All Rights Reserved                                                  *
*                                                                                                       *
*   FileName    :   protocol_def.h                                                                      *
*   Description :   协议库公用宏定义                                                                    *
*   Author      :   Valder-Fields                                                                                 *
*   Date        :                                                                                       *
*   Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description                                                *
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __PROTOCOL_DEF_H__
#define __PROTOCOL_DEF_H__


#if defined (WIN32)
#	if defined (LIB_PROTOCOL)
#		define	 PROTOCOL_API		__declspec(dllexport)
#	else
#		define	 PROTOCOL_API		__declspec(dllimport)
#	endif
#else
#	define	 PROTOCOL_API
#endif

//调试声明

#ifdef QT_DEBUG_MODE_PROTOCOL
#include <QDebug>
#include <QTime>
#define QDEBUG(str) QDebug()<<str<<QTime::currentTime();
#else
#define QDEBUG(str)
#endif

//协议解析模块独立宏定义
//INDEPENDENT_MODULE_COMM_PROTOCOL
#if defined INDEPENDENT_MODULE_COMM_PROTOCOL

#include <string.h>

//系统级宏
#define     FALSE                       0       //失败
#define     TRUE                        1       //成功

#ifndef     NULL
#define     NULL    0
#endif

//全局宏
#define     COMMON_STRLEN_32        32
#define     COMMON_STRLEN_64        64
#define     COMMON_STRLEN_128       128
#define     COMMON_STRLEN_256       256
#define     COMMON_STRLEN_512       512
#define     COMMON_STRLEN_1024      1024

//通讯业务
#define     COMM_SENDDING_LENTH           (1024*1024*5)   //socket单次发送缓存长度(稳定版为3M)

#define     COMM_RXBUF_LEN                (1024*1024*5)   //客户端接收缓存大小(10M) //todo //test :稳定版为5M
#define     COMM_TXBUF_LEN                (1024*1024*5)   //客户端发送缓存大小(10M) //todo //test :稳定版为5M

#define     COMM_FRAME_MAXNUM             81920           //业务接收单帧最大长度


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 数据结构
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct COMM_CHANNEL_RXBUF
{
    int     head;
    int     tail;

    int     onceFrameLen;                       //单次接收一帧长度
    unsigned char onceFrame[COMM_FRAME_MAXNUM]; //单次接收一帧数据
    unsigned char buf[COMM_RXBUF_LEN];
};

struct COMM_CHANNEL_TXBUF
{
    int     head;
    int     tail;

    int     onceBufLen;                         //单次发送一次数据长度
    unsigned char onceBuf[COMM_SENDDING_LENTH]; //单次发送一次数据
    unsigned char buf[COMM_TXBUF_LEN];
};

struct CLIENT_COMM
{
    int     connFlag;   //通道连通标志
    int     socket;     //通道SOCKET
    int     connTime;   //通道连接时刻
    int     port;       //服务器端口
    char    ipAddr[COMMON_STRLEN_32];   //服务器IP地址

    int     commProcInter;   //心跳检查间隔
    int     heartbeatInter;    //心跳发送间隔
    int     heartbeatTimeout; //心跳超时时间
    int     heartWaitCount;   //心跳等待计数

    COMM_CHANNEL_RXBUF rxbuf;  //接收buf
    COMM_CHANNEL_TXBUF txbuf;  //发送buf
};

#else

    #include "../../../../common/def/common_def.h"
    #include "../../../include/common/comm_def.h"

#endif





#endif //__PROTOCOL_DEF_H__
