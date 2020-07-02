/********************************************************************************************************
*                                       COMMSOCKET LIB                                                  *
*																										*
*                               (c) Copyright 2019                                                      *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   comm_base.h                                                                         *
*	Description	:	基础通讯功能                                                                          *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#ifndef __COMM_BASE_H__
#define __COMM_BASE_H__

#include "commsocket_def.h"


class COMMSOCKET_API CommBase
{
public:

#if defined(_WIN32)
    //初始化socket,程序启动时用(win)
    static void     SocketStartup();

    //清理socket,程序退出时用(win)
    static void     SocketCleanup();
#endif

    //关闭socket
    static int      CloseSocket(int sock, int retry = 5);

    //获取上一个socket错误码
    static int      GetLastError(int sockFlag = 0);
};

#endif //__COMM_BASE_H__
