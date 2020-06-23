/********************************************************************************************************
*                                       commmanager                                                     *
*																										*
*                                   (c) Copyright 2020                                                  *
*                                   All Rights Reserved                                                 *
*																										*
*	FileName	:   commservice.h                                                                         *
*	Description	:	通讯管理-服务                                                                         *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __COMMSERVICE_H__
#define __COMMSERVICE_H__

#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4302 )


#include "../common/communicator_def.h"

class CommManager;

class COMMUNICATOR_API CommService
{
public:
    CommService(CommManager *manager);
    ~CommService();

    int  StartCommProc();
    int  InitCommProc();
    int  StopCommProc();

private:
    void SocketStartup();
    int  GetLastError(int sock_flag);

private:
    CommManager *m_manager;
};

#endif //__COMMSERVICE_H__
