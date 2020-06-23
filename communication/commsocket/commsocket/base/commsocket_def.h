/********************************************************************************************************
*                                       COMMSOCKET LIB                                                  *
*																										*
*                                    (c) Copyright 2019                                                 *
*                                    All Rights Reserved                                                *
*																										*
*	FileName	:   commsocket_def.h                                                                    *
*	Description	:	通讯库公用宏定义                                                                       *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#ifndef __COMMSOCKET_DEF_H__
#define __COMMSOCKET_DEF_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <errno.h>
#include <time.h>

#if defined (WIN32)
#	if defined (LIB_COMMSOCKET)
#		define	 COMMSOCKET_API		__declspec(dllexport)
#	else
#		define	 COMMSOCKET_API		__declspec(dllimport)
#	endif
#else
#	define	 COMMSOCKET_API
#endif

#if defined(WIN32)
#	include <windows.h>
#	include <winsock.h>

#if defined(WIN32) && defined(_DEBUG) && defined(_MSC_VER) && (_MSC_VER <= 1200 )
#include <crtdbg.h>
#endif

#	define  COMMSOCK_EWOULDBLOCK		WSAEWOULDBLOCK
#	define  COMMSOCK_EINTR			WSAEINTR
#	define	COMMSOCK_NDELAY			O_NDELAY
#	define  COMMSOCK_EISCONN			WSAEISCONN
#	define  COMMSOCK_EINPROGRESS		WSAEINPROGRESS
#	define	COMMSOCK_EALREADY		WSAEALREADY

#elif defined(__unix)
#	include <netinet/in.h>
#	include <unistd.h>
#	include <pthread.h>
#	include <string.h>
#	include <sys/time.h>

#	define  SOCKET				int
#	define  SOCKET_ERROR			-1

#	if defined(__hpux) || defined(_AIX)
#		define  COMMSOCK_EWOULDBLOCK		EAGAIN
#		define  COMMSOCK_EINTR			EINTR
#		define	COMMSOCK_NDELAY			O_NONBLOCK
#	else
#		define  COMMSOCK_EWOULDBLOCK		EWOULDBLOCK
#		define  COMMSOCK_EINTR			EINTR
#		define	COMMSOCK_NDELAY			O_NDELAY
#	endif

#	if defined (__linux)
#		define sigset(A, B)			signal(A, B)
#   endif
#	if defined (__sun)
#		include <sys/errno.h>
#		include <arpa/nameser_compat.h>	//byte-order
#	endif
#	if defined (__hpux)
#		include <arpa/nameser_compat.h>	//byte-order
#	endif
#	if defined (_AIX)
#		include <arpa/nameser_compat.h>	//byte-order
#	endif
#endif


//socket通讯模块独立宏定义
//INDEPENDENT_MODULE_COMM_COMMSOCKET
#if defined INDEPENDENT_MODULE_COMM_COMMSOCKET

//系统级错误定义
#define     FALSE                       0       //失败
#define     TRUE                        1       //成功

#ifndef     NULL
#define     NULL    0
#endif

#else

    #include "../../../../common/def/common_def.h"

#endif

//base
#define     SET_TXBUF_MAX_SIZE_WIN      8192    //最大发送缓存
#define     SET_RXBUF_MAX_SIZE_WIN      8192    //最大接收缓存

//TCP
#define     OPT_NB_RECVERR_SLEEPTIME    10      //非阻塞模式下recv错误(EINTR/EWOULDBLOCK)等待时间
#define     OPT_NB_SENDERR_SLEEPTIME    10      //非阻塞模式下send错误(EINTR/EWOULDBLOCK)等待时间

//UDP
#define     COMMUDP_MODE_UNICASTSEND    1       //udp传输模式:单播发送
#define     COMMUDP_MODE_UNICASTRECV    2       //udp传输模式:单播接收
#define     COMMUDP_MODE_BROADCASTSEND  3       //udp传输模式:广播发送
#define     COMMUDP_MODE_BROADCASTRECV  4       //udp传输模式:广播接收
#define     COMMUDP_MODE_MULTICASTSEND  5       //udp传输模式:组播发送
#define     COMMUDP_MODE_MULTICASTRECV  6       //udp传输模式:组播接收

//通讯错误码 --- 通用
#define     COMM_ERR_SOCKID             -1      //socket id错误

#define     COMM_ERR_CREATESOCK         -200    //socket创建失败
#define     COMM_ERR_BIND               -201    //bind失败
#define     COMM_ERR_SETRCVBUF          -210    //设置RCVBUF失败
#define     COMM_ERR_SETSNDBUF          -211    //设置SNDBUF失败

//通讯错误码 --- tcp
#define     COMMTCP_ERR_READ            -301    //tcp读取错误
#define     COMMTCP_ERR_WRITE           -302    //tcp写入错误
#define     COMMTCP_ERR_LSNSOCK         -310    //创建监听错误

//通讯错误码 --- udp
#define     COMMUDP_ERR_READ            -450    //udp读取错误
#define     COMMUDP_ERR_WRITE           -451    //udp写错误

#define     COMM_ERR_REUSEADDR          -401    //设置REUSEADDR失败
#define     COMM_ERR_BROADCAST          -402    //设置BROADCAST失败
#define     COMM_ERR_MULTICAST_IF       -403    //设置组播发送IP失败
#define     COMM_ERR_MULTICAST_LOOP     -404    //设置组播回环失败
#define     COMM_ERR_ADD_MEMBERSHIP     -405    //加入组播组失败
#define     COMM_ERR_DROP_MEMBERSHIP    -406    //离开组播组失败



#endif //__COMMSOCKET_DEF_H__
