/********************************************************************************************************
*                                       commmanager                                                     *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   commmanager_def.h                                                                    *
*	Description	:	通讯管理宏定义                                                                        *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __COMMUNICATOR_DEF_H__
#define __COMMUNICATOR_DEF_H__

#if defined (_WIN32)
#	if defined (LIB_COMMUNICATOR)
#		define	 COMMUNICATOR_API		__declspec(dllexport)
#	else
#		define	 COMMUNICATOR_API		__declspec(dllimport)
#	endif
#else
#	define	 LIB_COMMUNICATOR
#endif

//系统级引用
#include <fcntl.h>
#include <sys/stat.h>
#include <malloc.h>
#include <time.h>

#if defined(_WIN32)
    #include <direct.h>
    #include <io.h>
    #include <assert.h>
    #include <signal.h>

    #include <windows.h>
    #include <winsock.h>
    #include <wincon.h>

    #if defined(_DEBUG) && defined(_MSC_VER) && (_MSC_VER <= 1200 )
        #include <crtdbg.h>
    #endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>

//通讯管理模块独立宏定义
//INDEPENDENT_MODULE_COMM_COMMUNICATOR
#if defined INDEPENDENT_MODULE_COMM_COMMUNICATOR

//系统级错误定义
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

//时间相关
#if defined(_WIN32) && !defined (CYGWIN)
typedef __int64 int64_t;
#else
typedef long long int64t;
#endif


//通信宏定义
#define     COMM_SENDDING_LENTH           (1024*1024*5)   //socket单次发送缓存长度(稳定版为3M)

#define     COMM_RXBUF_LEN                (1024*1024*5)   //客户端接收缓存大小(10M) //todo //test :稳定版为5M
#define     COMM_TXBUF_LEN                (1024*1024*5)   //客户端发送缓存大小(10M) //todo //test :稳定版为5M

#define     COMM_SINGLE_READ_SIZE         8192            //通道单次读取长度

#define     COMM_FRAME_MAXNUM             81920           //业务接收单帧最大长度

//协议层宏定义
#define     COMM_PROTOCOL_SimpleTlv       10000           //简单TLV协议

//模块输出帧类型
#define     COMM_FRAMETYPE_MessageCode    1000            //消息码帧

//模块自用帧类型
#define     COMM_FRAMETYPE_Heartbeat      1001            //心跳帧

//输出信息
#define     COMM_Code_Connect_Success     100             //连接成功
#define     COMM_Code_Connect_Fail        101             //连接失败
#define     COMM_Code_Connect_Break       102             //连接断开

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 流程
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define     COMM_READPROC_INTER           15       //读线程轮循间隔(ms)
#define     COMM_PARSEPROC_INTER          15       //解析线程轮循间隔(ms)
#define     COMM_FORWARDING_INTER         15       //转发线程轮循间隔(ms)
//#define     COMM_SAVEPROC_INTER           30      //保存线程轮循间隔(ms)
#define     COMM_TRANSACTIONPROC_INTER    500     //事务处理线程轮循间隔(ms)

#define     COMM_CLICOMM_NUM              5       //客户端通讯通道个数
#define     COMM_CLICOMM_DEFAULTID        0       //默认客户端通讯通道索引

#define     COMM_CLI_NUM                  30      //系统客户端最大个数
//#define     COMM_LISTEN_MAXNUM            10      //服务器监听个数(端口与程序映射最大个数)

#define     COMM_SRVCOMM_NUM              1       //服务器监听端口个数
#define     COMM_SRVROOM_NUM              5  //test 目前为:COMM_CLI_NUM 服务器房间最大个数(应等于同时训练的最大课程数,目前="客户端最大个数")

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 帧定义
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct MessageCode
{
    int code;    //通讯模块信息代码
};

struct Heartbeat
{
    int     exeId;
    char    state;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 结构体定义
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
    int     connFlag;         //通道连通标志
    int     socket;           //通道SOCKET
    int     connTime;         //通道连接时刻
    int     lockFlag;   //连接过程锁(因connectChannel非原子操作,所以存在多线程同时connectChannel,的可能,故禁止同一时刻多个线程同时使用连接过程)

    int     heartCheckInter;    //心跳检查间隔
    int     heartbeatTime;    //心跳发送间隔
    int     heartbeatTimeout; //心跳超时时间
    int     heartWaitCount;   //心跳等待计数

    int     port;       //服务器端口
    char    ipAddr[COMMON_STRLEN_32];   //服务器IP地址

    COMM_CHANNEL_RXBUF rxbuf;  //接收buf
    COMM_CHANNEL_TXBUF txbuf;  //发送buf
};


#else

    #include "../../../../common/def/common_def.h"
    #include "../../../include/common/comm_def.h"

#endif


#endif //__COMMUNICATOR_DEF_H__
