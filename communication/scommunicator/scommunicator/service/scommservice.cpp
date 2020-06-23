

#include "../../../commsocket/commsocket.h"
#include "../../scommunicator.h"

//模块内引用
#include "../common/scommunicator_def.h"
#include "../manager/scommmanager.h"
#include "scommservice.h"

DWORD		glo_threadid_accept;
DWORD		glo_threadid_read;
DWORD		glo_threadid_forwarding;
DWORD		glo_threadid_dispatch;
DWORD		glo_threadid_service;
DWORD		glo_threadid_userthread;

int	 glo_liveflag  = TRUE;
int  glo_threadnum = 0;

NETSRV_SRVDATA glo_server;
NETSRV_CLIDATA glo_clients[COMM_CLI_NUM];

char m_logfilePath[COMMON_STRLEN_512] = {0};

CommUdpSocket *commUdpSocket = NULL;

SCommService::SCommService(SCommManager *manager)
{
    m_manager = manager;
}

SCommService::~SCommService()
{

}

/**
 * @brief StartCommProc 启动服务端通讯流程
 * @return
 */
int SCommService::StartCommProc()
{
    bool ret = false;

    if (m_manager == NULL) {
        return FALSE;
    }

    SocketStartup();

    ret = InitCommProc();

    //系统数据
    if (ret == true) {
        glo_server.runData.startTime = (int)time(NULL);
    }

    return ret;
}

/**
 * @brief InitCommProc 通讯线程初始化
 * @return
 */
int SCommService::InitCommProc()
{
    //udp初始化
    commUdpSocket = new CommUdpSocket();
    if (commUdpSocket == NULL) {
        assert(!"new CommUdpSocket error!");
    }

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_acceptproc,       m_manager, 0, &glo_threadid_accept)		== NULL) {
        return false;
    }

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_readproc,         m_manager, 0, &glo_threadid_read)		== NULL) {
        return false;
    }

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_forwardingproc,   m_manager, 0, &glo_threadid_forwarding)	== NULL) {
        return false;
    }

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_dispatchproc,     m_manager, 0, &glo_threadid_dispatch)   == NULL) {
        return false;
    }

    if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_serviceproc,      m_manager, 0, &glo_threadid_service)   == NULL) {
        return false;
    }

    if (glo_server.userThreadFlag == 1) {
        if (CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)srv_userproc,      m_manager, 0, &glo_threadid_userthread)   == NULL) {
            return false;
        }
    }

    return true;
}

int  SCommService::StopCommProc()
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
void SCommService::SocketStartup()
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
int SCommService::GetLastError(int sock_flag)
{
    if (sock_flag == 0) {
        return ::GetLastError();
    }
    else {
        return WSAGetLastError();
    }
}



