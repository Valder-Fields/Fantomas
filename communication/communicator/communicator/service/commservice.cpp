


#include "../../communicator.h"
#include "../common/communicator_def.h"
#include "../manager/commmanager.h"
#include "commservice.h"


DWORD		glo_threadid_read;
DWORD		glo_threadid_forwarding;
DWORD		glo_threadid_dispatch;

int	 glo_liveflag  = TRUE;
int  glo_threadnum = 0;

CLIENT_COMM  glo_comm;

CommService::CommService(CommManager *manager)
{
    m_manager = manager;
}

CommService::~CommService()
{

}

/**
 * @brief StartCommProc 启动服务端通讯流程
 * @return
 */
int CommService::StartCommProc()
{
    bool ret = false;

    if (m_manager == NULL) {
        return FALSE;
    }

    SocketStartup();

    ret = InitCommProc();

    return ret;
}

/**
 * @brief InitCommProc 通讯线程初始化
 * @return
 */
int CommService::InitCommProc()
{
    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_readproc,         m_manager, 0, &glo_threadid_read)		== NULL) {
        return false;
    }

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_forwardingproc,   m_manager, 0, &glo_threadid_forwarding)	== NULL) {
        return false;
    }

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_dispatchproc,        m_manager, 0, &glo_threadid_dispatch)       == NULL) {
        return false;
    }

    return true;
}

int  CommService::StopCommProc()
{
    glo_liveflag = FALSE;
    while (glo_threadnum > 0) {
        Sleep(100);
    }

    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//基础功能函数
////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief SocketStartup 初始化SOCKET资源(WIN32)
 * @return
 */
void CommService::SocketStartup()
{
    WSACleanup();
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD( 2, 2 );
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        printf ("error: --> SocketStartup -- WSAStartup error no:%d ... [%d]\n", GetLastError(1), __LINE__);
    }
    else {
        if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wHighVersion) != 2) {
            WSACleanup();
        }
    }
}

/**
 * @brief GetLastError 获得当前最后一个错误值
 * @return
 */
int CommService::GetLastError(int sock_flag)
{
    if (sock_flag == 0) {
        return ::GetLastError();
    }
    else {
        return WSAGetLastError();
    }
}



