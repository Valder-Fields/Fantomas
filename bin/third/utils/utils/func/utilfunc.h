/********************************************************************************************************
*                                       utils                                                           *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   utilfunc.h                                                                           *
*	Description	:	提供公共方法                                                                          *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/


#ifndef __UTILFUNC_H__
#define __UTILFUNC_H__


#include <stdio.h>
#include <time.h>

#include "../common/utils_def.h"
#include "../communication/include/common/comm_def.h"

class UTILS_API UtilFunc
{
public:
    UtilFunc();
    ~UtilFunc();

public:
    //系统
    //获取应用程序所在全路径
    static void GetApplicationDirPath(char *path);

public:
    //时间
    //获取当前时间(ms)
    static int64_t GetSysTimeMS();

    //tm格式时间
    static void GetCurTimeInfo(CommonTimeInfo &t, const int64_t &time_ms);

public:
    //文件操作
    //
    static int  GetFileLength(FILE *fp, const char *fileName, int &fileLength);

    //
    static bool CreateDirPath(const char *dir);

    //
    static int  GetFileNameByFullFilePath(const char *fullFilePath, char *fileName);

    //
    static int  ReadFile(FILE *fp, const char *fileName, unsigned char *buffer, int bufferLen);

    //
    static bool DelFile(const char *fileName);

    //保存文件
    static int  SaveFile(const char *fileName, char *buffer, int bufferLen, int mode = FILEOPER_MODE_APPEND);

    //获取路径目录
    static int  GetFileDirInfo(const char *path, FRM_CallFileDirRet &v);

public:
    static int  PrintHexStr(char *str, int size);
    static int  PrintDexStr(char *str, int size);

};


#endif
