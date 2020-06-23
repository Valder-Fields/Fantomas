/********************************************************************************************************
*                                       NetServer                                                       *
*																										*
*                               (c) Copyright 2020,   HYJG                                              *
*                                  All Rights Reserved                                                  *
*																										*
*	FileName	:   pubfunc.h                                                                           *
*	Description	:	公共函数声明                                                                          *
*	Author		:	Valder-Fields																					*
*	Date		:           																			*
*	Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description												*
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#ifndef __PUBFUNC_H__
#define __PUBFUNC_H__

#include "global_def.h"

#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDataStream>
#include <QDebug>
#include <QTime>
#include <QDateTime>
#include <QFile>
#include <QCoreApplication>
#include <QDomDocument>
#include <QFile>

class PubFunc
{
public:
    PubFunc() {}
    ~PubFunc() {}

public:
    //XML
    static int     ReadConfXML();
    static int     ReadConf_Client(QDomElement &client, int cliIdx);
    static int     ReadConf_Server(QDomElement &server);

};

#endif // __PUBFUNC_H__
