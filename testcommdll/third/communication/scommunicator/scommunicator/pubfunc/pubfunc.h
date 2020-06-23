#ifndef __PUBFUNC_H__
#define __PUBFUNC_H__

#include <QCoreApplication>
#include <QDomElement>
#include <QFile>
#include <QDebug>

class GloFunc
{
public:
    GloFunc();
    ~GloFunc();

public:
    //全局初始化
    static int  InitGloFunc();

    //初始化全部公共数据
    static int  InitGloData();

    //XML
    static int  ReadConfXML();
    static int  ReadConf_Client(QDomElement &client, int cliId);
    static int  ReadConf_Server(QDomElement &server);

};

#endif // __PUBFUNC_H__
