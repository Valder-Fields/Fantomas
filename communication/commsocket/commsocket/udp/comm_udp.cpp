

#include "../../../../utils/utils.h"

#include "comm_udp.h"

CommUdpSocket::CommUdpSocket()
{
    memset(&m_localAddr, 0, sizeof(m_localAddr));
    memset(&m_group, 0, sizeof(m_group));

    memset(m_destIp, 0, sizeof(char)*256);
    memset(m_localIp, 0, sizeof(char)*256);

    m_mode = 0;
    m_port = -1;
    m_socket = -1;
    m_logFlag = FALSE;

    m_rcvbuf_size = 655360;
    m_sendbuf_size = 655360;
}

CommUdpSocket::~CommUdpSocket()
{
    Close();
}

int CommUdpSocket::SetDestIp(char *ip)
{
    if (ip == NULL) {
        return FALSE;
    }

    if (strlen(ip) >= 256) {
        return FALSE;
    }

    sprintf(m_destIp, "%s", ip);

    return TRUE;
}

int CommUdpSocket::SetLocalIp(char *ip)
{
    if (ip == NULL) {
        return FALSE;
    }

    if (strlen(ip) >= 256) {
        return FALSE;
    }

    sprintf(m_localIp, "%s", ip);

    return TRUE;
}

int CommUdpSocket::Init(UdpSocketCfg &cfg)
{
    SetMode(cfg.mode);
    SetPort(cfg.port);
    SetDestIp(cfg.destIp);

    int ret = FALSE;

    if (m_socket > 0) {
        Close();
    }

    m_socket = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (m_socket < 0) {
        perror("Create datagram socket error");
        return COMM_ERR_CREATESOCK;
    }
    else {
        printf("Create datagram socket....OK.\n");
    }

    //接收发送缓存
    if (m_rcvbuf_size > 0) {
        ret = setsockopt(m_socket,SOL_SOCKET,SO_RCVBUF,(char *)&m_rcvbuf_size, sizeof(m_rcvbuf_size));

        if (ret < 0) {
            return COMM_ERR_SETRCVBUF;
        }
    }
    if (m_sendbuf_size > 0) {
        ret = setsockopt(m_socket,SOL_SOCKET,SO_SNDBUF,(char *)&m_sendbuf_size, sizeof(m_sendbuf_size));
        if (ret < 0) {
            return COMM_ERR_SETSNDBUF;
        }
    }

    //网络地址复用
    int reuse = 1;
    ret = setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
    if (ret < 0) {
        perror("Set SO_REUSEADDR error");
        Close();
        return COMM_ERR_REUSEADDR;
    }
    else {
        printf("Set SO_REUSEADDR...OK.\n");
    }

    //阻塞模式
    unsigned long noblock_flag = TRUE;
    ioctlsocket(m_socket, FIONBIO, &noblock_flag);

    //其他设置
    switch (m_mode) {
    //广播发送
    case COMMUDP_MODE_BROADCASTSEND:
    {
        ret = EnableBroadcast(true);
        if (ret != TRUE) {
            return ret;
        }

        memset((char *)&m_destAddr, 0, sizeof(m_destAddr));

        m_destAddr.sin_family = AF_INET;
        m_destAddr.sin_addr.s_addr = inet_addr(m_destIp);
        //待确定
        //m_destAddr.sin_addr.s_addr = INADDR_BROADCAST;
        m_destAddr.sin_port = htons(m_port);
    }
        break;

    //广播接收
    case COMMUDP_MODE_BROADCASTRECV:
    {
        //绑定
        memset((char *)&m_localAddr, 0, sizeof(m_localAddr));
        m_localAddr.sin_family = AF_INET;
        m_localAddr.sin_port = htons(m_port);
        m_localAddr.sin_addr.s_addr = INADDR_ANY;

        ret = bind(m_socket, (struct sockaddr*)&m_localAddr, sizeof(struct sockaddr));
        if (ret < 0) {
            perror("Bind datagram socket error");
            Close();
            return COMM_ERR_BIND;
        }
        else {
            printf("Bind datagram socket...OK.\n");
        }
    }
        break;

    //组播发送
    case COMMUDP_MODE_MULTICASTSEND:
    {
        //发送地址
        memset((char *)&m_destAddr, 0, sizeof(m_destAddr));

        //设置组播发送地址
        m_destAddr.sin_family = AF_INET;
        m_destAddr.sin_addr.s_addr = inet_addr(m_destIp);//GROUP_IP 第一个字节<=239 后面3个字节用来区别分组 不是掩码
        m_destAddr.sin_port = htons(m_port);

        /*
        //设置组播默认网络接口
        struct in_addr localIp;
        //localIp.s_addr = inet_addr(m_localIp);//自己的ip,可以和目标不在一个网，但设置不同的网段。
        localIp.s_addr = htonl(INADDR_ANY);//自己的ip,可以和目标不在一个网，但设置不同的网段。

        ret = setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localIp, sizeof(localIp));
        if (ret < 0) {
            return COMM_ERR_MULTICAST_IF;
        }*/

        //禁止回环
        unsigned char loop = 0;
        ret = setsockopt(m_socket, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop));
        if (ret < 0) {
            return COMM_ERR_MULTICAST_LOOP;
        }
    }
        break;

    //组播接收
    case COMMUDP_MODE_MULTICASTRECV:
    {
        //设置本地地址
        memset((char *)&m_localAddr, 0, sizeof(m_localAddr));
        m_localAddr.sin_family = AF_INET;
        m_localAddr.sin_port = htons(m_port);
        m_localAddr.sin_addr.s_addr = INADDR_ANY;

        //绑定
        ret = bind(m_socket, (struct sockaddr*)&m_localAddr, sizeof(struct sockaddr));
        if (ret < 0) {
            perror("Bind datagram socket error");
            Close();
            return COMM_ERR_BIND;
        }
        else {
            printf("Bind datagram socket...OK.\n");
        }

        //加入组播组
        ret = EnableMulticastGroup(true);
        if (ret != TRUE) {
            return ret;
        }
    }
        break;

    //单播发送
    case COMMUDP_MODE_UNICASTSEND:
    {
        memset((char *)&m_destAddr, 0, sizeof(m_destAddr));

        m_destAddr.sin_family = AF_INET;
        m_destAddr.sin_addr.s_addr = inet_addr(m_destIp);
        m_destAddr.sin_port = htons(m_port);
    }
        break;

    //单播接收
    case COMMUDP_MODE_UNICASTRECV:
    {
        //绑定
        memset((char *)&m_localAddr, 0, sizeof(m_localAddr));
        m_localAddr.sin_family = AF_INET;
        m_localAddr.sin_port = htons(m_port);
        m_localAddr.sin_addr.s_addr = INADDR_ANY;
        ret = bind(m_socket, (struct sockaddr*)&m_localAddr, sizeof(struct sockaddr));
        if (ret < 0) {
            perror("Bind datagram socket error");
            Close();

            if (m_logFlag) {
                //char text[256];
                //sprintf(text, "%s", "UNICASTRECV bind error \n");
                //UtilFunc::SaveFile(m_logPath, text, (int)strlen(text));
            }

            return COMM_ERR_BIND;
        }
        else {
            printf("Bind datagram socket...OK.\n");
        }
    }
        break;

    default:
        break;
    }

    ret = TRUE;

    return ret;
}

int CommUdpSocket::Close()
{
    if (m_socket > 0) {
        switch (m_mode) {
        case COMMUDP_MODE_BROADCASTSEND:
        {
            EnableBroadcast(false);
        }
            break;

        case COMMUDP_MODE_BROADCASTRECV:
        {

        }
            break;

        case COMMUDP_MODE_MULTICASTSEND:
        {

        }
            break;

        case COMMUDP_MODE_MULTICASTRECV:
        {
            EnableMulticastGroup(false);
        }
            break;

        case COMMUDP_MODE_UNICASTSEND:
        {

        }
            break;

        case COMMUDP_MODE_UNICASTRECV:
        {

        }
            break;

        default:
            break;
        }

        CommBase::CloseSocket(m_socket);
    }

    return TRUE;
}

int CommUdpSocket::Read(char *buf, int bufLen)
{
    if (buf == NULL || m_socket < 0) {
        return FALSE;
    }

    int fromAddrLen = sizeof(struct sockaddr_in);
    int rxnum = recvfrom(m_socket, buf, bufLen, 0, (struct sockaddr *)&m_fromAddr, &fromAddrLen);

    if(rxnum==
#if defined(_WIN32)
        SOCKET_ERROR
#elif defined(__unix)
        -1
#endif
       ) {
        return COMMUDP_ERR_READ;
    }

    return rxnum;
}

int CommUdpSocket::Write(char *buf, int bufLen)
{
    if (buf == NULL || m_socket < 0) {
        return FALSE;
    }

    int txnum = sendto(m_socket, (char *)buf, bufLen, 0, (struct sockaddr *)&m_destAddr, sizeof(struct sockaddr));
    if (txnum ==
#if defined(_WIN32)
        SOCKET_ERROR
#elif defined(__unix)
        -1
#endif
    ) {
        return COMMUDP_ERR_WRITE;
    }

    return txnum;
}

void CommUdpSocket::SetLogPath(char *fullPath)
{
    if (fullPath == NULL) {
        return;
    }


}

int CommUdpSocket::EnableBroadcast(bool enable)
{
    int ret = FALSE;
    if (enable) {
#if defined(_WIN32)
        int temp = 1;
        ret = setsockopt(m_socket,SOL_SOCKET,SO_BROADCAST,(char *)&temp,sizeof(temp));
        if (ret < 0) {
            return COMM_ERR_BROADCAST;
        }

#elif defined(__unix)
        fcntl(sockid,F_SETFL,O_NDELAY);
#endif
    }
    else {
#if defined(_WIN32)
        int temp = 0;
        ret = setsockopt(m_socket,SOL_SOCKET,SO_BROADCAST,(char *)&temp,sizeof(temp));
        if (ret < 0) {
            return COMM_ERR_BROADCAST;
        }
#elif defined(__unix)

#endif
    }

    return TRUE;
}

int CommUdpSocket::EnableMulticastGroup(bool enable)
{
    int ret = FALSE;

    if (enable) {
        //加入组播组
        m_group.imr_multiaddr.s_addr = inet_addr(m_destIp);//加入的组播组
        //m_group.imr_interface.s_addr = inet_addr(m_localIp);//本地的ip
        m_group.imr_interface.s_addr = htonl(INADDR_ANY);//本地的任意ip

        //ret = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m_group, sizeof(m_group));
        //ret = setsockopt(m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&m_group, sizeof(m_group));
        ret = setsockopt(m_socket, IPPROTO_IP, 12, (char *)&m_group, sizeof(m_group)); //winsock.h winsock2.h版本不同导致IP_ADD_MEMBERSHIP宏定义不识别=5的情况,新版本IP_ADD_MEMBERSHIP=12,在暂
																					   //时没统一所有socket库版本时,先使用数字定义
        if (ret < 0) {
            perror("Add multicast m_group error");
            //int err = CommBase::GetLastError();
            return COMM_ERR_ADD_MEMBERSHIP;
        }
        else {
            printf("Add multicast m_group...OK.\n");
        }
    }
    else {
        //退出组播组
        ret = setsockopt(m_socket, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&m_group, sizeof(m_group));
        if (ret < 0) {
            perror("Drop multicast m_group error");
            return COMM_ERR_DROP_MEMBERSHIP;
        }
        else {
            printf("Drop multicast m_group...OK.\n");
        }
    }

    return TRUE;
}

