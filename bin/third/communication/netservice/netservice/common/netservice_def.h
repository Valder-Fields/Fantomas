/********************************************************************************************************
*                                       commmanager                                                     *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   commmanager_def.h                                                                    *
*	Description	:   通讯管理宏定义                                                                        *
*	Author		:   Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __NETSERVICE_DEF_H__
#define __NETSERVICE_DEF_H__

#if defined (_WIN32)
#	if defined (LIB_NETSERVICE)
#		define	 NETSERVICE_API		__declspec(dllexport)
#	else
#		define	 NETSERVICE_API		__declspec(dllimport)
#	endif
#else
#	define	 LIB_NETSERVICE
#endif


#endif //__NETSERVICE_DEF_H__
