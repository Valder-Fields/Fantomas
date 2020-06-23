/********************************************************************************************************
*                                       COMMSOCKET LIB                                                  *
*																										*
*                               (c) Copyright 2019                                                      *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   comm_udp.h                                                                          *
*	Description	:	udp通讯                                                                              *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#include "../base/comm_base.h"


//#define  GROUP_IP	"239.255.0.1"//224.0.0.0-239.255.255.255

struct UdpSocketCfg
{
    int            mode;        //模式
    unsigned short port;        //端口
    char           destIp[256]; //目的地址
};


//组播组信息,理论上一个客户端可以加入多个组播组,但目前考虑一个客户端生成一个CommUdpProc对象,故暂时未使用本结构体
struct MulticastGroupInfo
{
    int    groupNo;       //组播组的序号

    int    groupPort;     //组播组端口
    char   groupIp[256];  //组播组的IP
};

class COMMSOCKET_API CommUdpSocket
{
public:
    CommUdpSocket();
    ~CommUdpSocket();

    void    SetMode(int mode) { m_mode = mode; }
    void    SetPort(int port) { m_port = port; }

    int     SetDestIp(char *ip);
    int     SetLocalIp(char *ip);

    int     Init(UdpSocketCfg &cfg);
    int     Close();

    //读(返回值:读到的字节数)
    int     Read(char *buf, int bufLen);

    //写(返回值:发送成功的字节数)
    int     Write(char *buf, int bufLen);

    //调试用
    int     GetSocket() { return m_socket; }

    //log相关
    void    SetLogFlag(int flag) { m_logFlag = flag; }
    void    SetLogPath(char *fullPath);

private:
    //设置广播(允许/禁止)
    int     EnableBroadcast(bool enable);

    //设置组播组(加入/退出)
    int     EnableMulticastGroup(bool enable);

private:
    struct  sockaddr_in m_localAddr;   //本地的socket设置(接收方绑定)
    struct  sockaddr_in m_fromAddr;    //收到的socket设置(接收方接收)
    struct  sockaddr_in m_destAddr;    //目的socket设置(发送方发送)

    unsigned short m_port;     //端口

    char    m_localIp[256];    //本地IP地址(发送/接收方设置组播参数使用)
    char    m_destIp[256];     //目的IP地址(组播发送方发送时使用/接收方设置组播参数使用)

    struct  ip_mreq m_group;   //组播参数(接收方使用)

    int     m_socket;          //

    int     m_rcvbuf_size;     //接收缓存长度
    int     m_sendbuf_size;    //发送缓存长度

    int     m_mode;            //传输模式(组播/广播)

    int     m_logFlag;
    char    m_logPath[512];
};
