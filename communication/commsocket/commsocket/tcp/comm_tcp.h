/********************************************************************************************************
*                                       COMMSOCKET LIB                                                  *
*																										*
*                               (c) Copyright 2019                                                      *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   comm_tcp.h                                                                          *
*	Description	:	tcp通讯                                                                              *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#include "../base/comm_base.h"

class COMMSOCKET_API CommTcpSocket
{
public:
    CommTcpSocket();
    ~CommTcpSocket();

    //建立tcp连接(参数: hostname:目标IP地址 port:目标端口 noblock_flag: 1:noblock 0:block 返回值:socket id)
    static int      ConnectChannel(char *hostname, int port, unsigned long noblock_flag = TRUE);

    //读取socket(retry:单次读取产生允许异常后的重复次数 返回值:读取字节数)
    static int      Read(int sock,char *buf,int size,int retry = 0);

    //写入socket(retry:单次写入产生允许异常后的重复次数 返回值:写入字节数 )
    static int      Write(int sock,char *buf,int size,int retry = 2);

    //监听端口(返回值:listen socket id)
    static int      ListenPort(int port);

    //accept(srvid:监听socket 返回值:accept的socket id)
    static int      Accept(int srvid, char *clientip, int *clientport);
};
