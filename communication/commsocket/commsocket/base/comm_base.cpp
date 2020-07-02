#include "comm_base.h"

#if defined(_WIN32)
//+++++++++++++++++++++++++ FUNCTION DESCRIPTION ++++++++++++++++++++++++++++++
//
// NAME        : SocketStartup
//
// DESCRIPTION : 初始化SOCKET资源(_WIN32)
//
// COMPLETION
// INPUT       :
// OUTPUT      :
// STATUS      : void
//
//-----------------------------------------------------------------------------
void CommBase::SocketStartup()
{
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD( 2, 2 );
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        printf ("libcomm error: --> SocketStartup -- WSAStartup error no:%d ... [%d]\n", GetLastError(1), __LINE__);
    }
    else {
        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wHighVersion) != 2) {
            SocketCleanup();
        }
    }
}

//+++++++++++++++++++++++++ FUNCTION DESCRIPTION ++++++++++++++++++++++++++++++
//
// NAME        : SocketClenup
//
// DESCRIPTION : 释放SOCKET资源(_WIN32)
//
// COMPLETION
// INPUT       :
// OUTPUT      :
// STATUS      : void
//
//-----------------------------------------------------------------------------
void CommBase::SocketCleanup()
{
    WSACleanup();
}
#endif

int CommBase::CloseSocket(int sock, int retry)
{
    if (sock > 0) {
        unsigned long noblock_flag = FALSE;
        ioctlsocket(sock, FIONBIO, &noblock_flag);
        for (int i=0; i<retry; i++)	{

#if defined (_WIN32)
            int ret = closesocket(sock);
#elif defined (__unix)
            int ret = close(sock);
#endif
            if (ret == 0)  break;
        }
    }

    return TRUE;
}

//+++++++++++++++++++++++++ FUNCTION DESCRIPTION ++++++++++++++++++++++++++++++
//
// NAME        : GetLastError
//
// DESCRIPTION : 获得当前运行的最后一个错误值
//
// COMPLETION
// INPUT       :
// OUTPUT      :
// STATUS      : int
//
//-----------------------------------------------------------------------------
int CommBase::GetLastError(int sockFlag)
{
#if defined (__unix)
    #if   defined (__alpha)
          return _Geterrno();
    #else
          return errno;
    #endif
#elif defined(_WIN32)
    if (sockFlag == 0) {
        return ::GetLastError();
    }
    else {
        return WSAGetLastError();
    }
#endif
}
