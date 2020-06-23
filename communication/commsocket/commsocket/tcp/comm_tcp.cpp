#include "comm_tcp.h"

CommTcpSocket::CommTcpSocket()
{

}

CommTcpSocket::~CommTcpSocket()
{

}

int CommTcpSocket::ConnectChannel(char *hostname, int port, unsigned long noblock_flag)
{
    if (hostname == NULL || port <= 0) return FALSE;

    char   buf[10];
    int    sockid;
    struct sockaddr_in txaddr;
    txaddr.sin_family = AF_INET;

    txaddr.sin_port        = htons(port);
    txaddr.sin_addr.s_addr = inet_addr(hostname);
    if (txaddr.sin_addr.s_addr == INADDR_NONE) {
        return FALSE;
    }

    if((sockid=(int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<=0) {
        return FALSE;
    }

    ioctlsocket(sockid,FIONBIO, &noblock_flag);

    fd_set fd;
    struct timeval tv;
    if ((::connect(sockid,(struct sockaddr *)&txaddr,sizeof(txaddr))==0) || (GetLastError()==COMMSOCK_EISCONN)) {

        unsigned long noblock_flag = TRUE;
        ioctlsocket(sockid,FIONBIO,&noblock_flag);

        tv.tv_sec  = 0;
        tv.tv_usec = 1000;
        setsockopt(sockid,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv));
        setsockopt(sockid,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv));

        char keep_alive=TRUE;
        setsockopt(sockid,SOL_SOCKET,SO_KEEPALIVE,(char *)&keep_alive,sizeof(keep_alive));

        int temp = SET_TXBUF_MAX_SIZE_WIN;
        setsockopt(sockid,SOL_SOCKET,SO_SNDBUF,(char *) &temp,sizeof(temp));
        temp = SET_RXBUF_MAX_SIZE_WIN;
        setsockopt(sockid,SOL_SOCKET,SO_RCVBUF,(char *) &temp,sizeof(temp));

        if (recv(sockid, buf, 2, MSG_PEEK) < 0) {
            if(GetLastError() != COMMSOCK_EWOULDBLOCK && GetLastError() != COMMSOCK_EINTR) {
                CommBase::CloseSocket(sockid);
                return FALSE;
            }
        }
        return sockid;
    }
    else {
        if ((GetLastError()==EINPROGRESS)||(GetLastError()==WSAEWOULDBLOCK)||(GetLastError()==EALREADY)) {

            tv.tv_sec = 1;    //1s
            tv.tv_usec= 0;

            int ret;
            {
                FD_ZERO(&fd);
                FD_SET(sockid, &fd);
                ret=select(FD_SETSIZE,0,&fd,0,&tv);
                if(ret>0)
                {
                    tv.tv_sec =  0;
                    tv.tv_usec =1000;
                    setsockopt(sockid,SOL_SOCKET,SO_SNDTIMEO,(char *)&tv,sizeof(tv));
                    setsockopt(sockid,SOL_SOCKET,SO_RCVTIMEO,(char *)&tv,sizeof(tv));

                    char keep_alive=TRUE;
                    setsockopt(sockid,SOL_SOCKET,SO_KEEPALIVE,(char *)&keep_alive,sizeof(keep_alive));

                    int temp = SET_TXBUF_MAX_SIZE_WIN;
                    setsockopt(sockid,SOL_SOCKET,SO_SNDBUF,(char *) &temp,sizeof(temp));
                    temp = SET_RXBUF_MAX_SIZE_WIN;
                    setsockopt(sockid,SOL_SOCKET,SO_RCVBUF,(char *) &temp,sizeof(temp));

                    if (recv(sockid,buf,2,MSG_PEEK) < 0) {
                        if (GetLastError() != COMMSOCK_EWOULDBLOCK && GetLastError() != COMMSOCK_EINTR) {
                            CommBase::CloseSocket(sockid);
                            return FALSE;
                        }
                    }
                    return sockid;
                }
            }

            CommBase::CloseSocket(sockid);
            return FALSE;
        }
    }

    CommBase::CloseSocket(sockid);
    return FALSE;
}

int CommTcpSocket::Read(int sock, char *buf,int size,int retry)
{
    if (sock<0)		return COMM_ERR_SOCKID;

    if (size==0)	return COMMTCP_ERR_READ;
    if (buf==NULL)	return COMMTCP_ERR_READ;

    int rxntime = 0;
    int rxn,err,rxnum = 0;

    while (TRUE) {
        rxn = recv(sock,buf+rxnum,size-rxnum,0);
        if (rxn==0) break;

        if (rxn<0) {
            err=GetLastError();
            if (err==COMMSOCK_EWOULDBLOCK || err==COMMSOCK_EINTR) {
                if ((rxntime++) >= retry) return 0;
                Sleep(OPT_NB_RECVERR_SLEEPTIME);
                continue;
            }
            else {
                //可在此判断网络断线
                break;
            }
        }

        rxnum+=rxn;
        return rxnum;
    }

    return rxnum;
}

/**
 * @brief CommProc::WriteSocket
 * @param sock
 * @param buf
 * @param size
 * @param retry
 * @return
 */
int CommTcpSocket::Write(int sock,char *buf,int size,int retry)
{
    if (sock<0) return COMM_ERR_SOCKID;

    if (size==0)		return COMMTCP_ERR_WRITE;
    if (buf==NULL)		return COMMTCP_ERR_WRITE;

    int txntime = 0;
    char *ptr = (char *)buf;
    int txn, err, txsize;

    txsize = 0;
    while (TRUE) {
        txn = send(sock,ptr,size,0);
        if (txn==0) break;
        if (txn<=0) {
            err=GetLastError();
            if (err==COMMSOCK_EWOULDBLOCK || err==COMMSOCK_EINTR) {
                if ((txntime++)>=retry) break;
                Sleep(OPT_NB_SENDERR_SLEEPTIME);
                continue;
            }
            else {
                //可在此判断网络断线
                break;
            }
        }

        txntime = 0;
        txsize += txn;
        size -= txn;
        ptr += txn;
        if (size>0) continue;

        return txsize;
    }

    return txsize;
}

int CommTcpSocket::ListenPort(int port)
{
    if (port < 0 || port > 65536) {
        return COMMTCP_ERR_LSNSOCK;
    }

    int temp;
    int ssockid = (int)INVALID_HANDLE_VALUE;

    if((ssockid=(int)socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0) {
        return COMMTCP_ERR_LSNSOCK;
    }

    unsigned long noblock_flag = TRUE;
    ioctlsocket(ssockid, FIONBIO, &noblock_flag);

    temp = SET_TXBUF_MAX_SIZE_WIN;
    if (setsockopt(ssockid, SOL_SOCKET, SO_RCVBUF, (char *)&temp, sizeof(temp)) < 0) printf("SO_RCVBUF error\n");
    temp = SET_RXBUF_MAX_SIZE_WIN;
    if (setsockopt(ssockid, SOL_SOCKET, SO_SNDBUF, (char *)&temp, sizeof(temp)) < 0) printf("SO_SNDBUF error\n");

    temp = 1;
    if (setsockopt(ssockid, SOL_SOCKET, SO_REUSEADDR, (char *)&temp, sizeof(temp)) < 0) printf("SO_REUSEADDR error\n");

    struct timeval tv;
    tv.tv_sec  = 1; //8
    tv.tv_usec = 0;

    int bindcnt = 0;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    while (bind(ssockid, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        if (bindcnt++ < 10) {
            Sleep(1000);
        }
        else break;
    }

    if ((bindcnt >= 10) || (listen(ssockid, 5) == SOCKET_ERROR)) {

        closesocket(ssockid);

        ssockid = (int)INVALID_HANDLE_VALUE;
    }

    return ssockid;
}

int CommTcpSocket::Accept(int srvid, char *clientip, int *clientport)
{
    int sockid = (int)INVALID_HANDLE_VALUE;

    if (srvid    <= (int)INVALID_HANDLE_VALUE)  return sockid;
    if (clientip == NULL || clientport == NULL) return sockid;

    fd_set rxd;
    char   *pipaddr = NULL;
    struct timeval tv;
    struct sockaddr_in addr;
    int    nb, tp;

    tv.tv_sec  = 0;
    tv.tv_usec = 100;
    FD_ZERO(&rxd);
    FD_SET(srvid, &rxd);
    nb = select(FD_SETSIZE, &rxd, 0, 0, &tv);
    if (nb <= 0) return sockid;		//如果服务器目前处于可以接受客户请求的状态

    tp = sizeof(addr);
    for (; ;) {
        if ((sockid = (int)accept(srvid, (struct sockaddr *)&addr, (int *)&tp)) <= 0) {
            if (GetLastError() == COMMSOCK_EINTR) continue;
        }

        unsigned long noblock_flag = TRUE;
        ioctlsocket(sockid, FIONBIO, &noblock_flag);

        break;
    }

    if (sockid > 0) {
        pipaddr = inet_ntoa(addr.sin_addr);
        if (pipaddr == NULL) {
            return sockid;
        }
        sprintf(clientip, "%s", pipaddr);
        *clientport = ntohs(addr.sin_port);
    }

    return sockid;
}

