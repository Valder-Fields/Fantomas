/********************************************************************************************************
*                                       scommmanager                                                    *
*																										*
*                                   (c) Copyright 2020                                                  *
*                                   All Rights Reserved                                                 *
*																										*
*	FileName	:   scommservice.h                                                                      *
*	Description	:	通讯管理-服务                                                                         *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __SCOMMSERVICE_H__
#define __SCOMMSERVICE_H__

#pragma warning ( disable : 4311 )
#pragma warning ( disable : 4302 )


#include "../common/scommunicator_def.h"

class SCommManager;

class SCOMMUNICATOR_API SCommService
{
public:
    SCommService(SCommManager *manager);
    ~SCommService();

    int  StartCommProc();
    int  InitCommProc();
    int  StopCommProc();

private:
    void SocketStartup();
    int  GetLastError(int sock_flag);

private:
    SCommManager *m_manager;
};

#endif //__SCOMMSERVICE_H__
