/********************************************************************************************************
*                                       communication                                                   *
*                                                                                                       *
*                               (c) Copyright 2020                                                        *
*                                  All Rights Reserved                                                  *
*                                                                                                       *
*   FileName    :   common_def.h                                                                        *
*   Description :   通讯系统公共宏定义                                                                     *
*   Author      :   Valder-Fields                                                                                 *
*   Date        :                                                                                       *
*   Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description                                                *
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

//系统级宏
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
#if defined(WIN32) && !defined (CYGWIN)
typedef __int64 int64_t;
#else
typedef long long int64t;
#endif

//系统路径
#define     SYSTEM_PATH_CONFIG   "/config"
#define     SYSTEM_PATH_LOG      "/log"
#define     SYSTEM_PATH_PROCDB   "/procdb"
#define     SYSTEM_PATH_FILE     "/file"

//文件操作
#define     FILEOPER_MODE_APPEND    1 //"ab+"
#define     FILEOPER_MODE_NEW       2 //"wb+"

////////////////////////////////////////////////////////////////////////////////////////////////////////
///
////////////////////////////////////////////////////////////////////////////////////////////////////////

//通用时间信息
struct CommonTimeInfo
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int microSec;
};




#endif //__COMMON_DEF_H__
