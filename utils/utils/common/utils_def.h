/********************************************************************************************************
*                                          utils                                                        *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   utils_interface.h                                                                   *
*	Description	:	提供公共方法,对外C接口                                                                 *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __UTILS_DEF_H__
#define __UTILS_DEF_H__


#if defined (_WIN32)
#	if defined (LIB_UTILS)
#		define	 UTILS_API		__declspec(dllexport)
#	else
#		define	 UTILS_API		__declspec(dllimport)
#	endif
#else
#	define	 LIB_COMMUNICATOR
#endif

//模块独立宏定义
//INDEPENDENT_MODULE_UTILS
#if defined INDEPENDENT_MODULE_UTILS

//系统级错误定义
#define     FALSE                       0       //失败
#define     TRUE                        1       //成功

#ifndef     NULL
#define     NULL    0
#endif

//全局宏
#define     COMMON_STRLEN_32        32
#define     COMMON_STRLEN_64        64
#define     COMMON_STRLEN_128       128
#define     COMMON_STRLEN_256       256
#define     COMMON_STRLEN_512       512
#define     COMMON_STRLEN_1024      1024

//时间相关
#if defined(_WIN32) && !defined (CYGWIN)
typedef __int64 int64_t;
#else
typedef long long int64t;
#endif

#else

    #include "../../../common/def/common_def.h"

#endif


#endif //__COMMUNICATOR_INTERFACE_H__
