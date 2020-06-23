
#include "../common/communicator_def.h"
#include "../../../../utils/utils.h"
#include "pubfunc.h"

extern CLIENT_COMM  glo_comm;

GloFunc::GloFunc()
{
 
}

GloFunc::~GloFunc()
{

}

int GloFunc::InitGloFunc()
{
    //公共数据初始化
    InitGloData();

    return TRUE;
}

int GloFunc::InitGloData()
{
    memset(&glo_comm, 0, sizeof(CLIENT_COMM));

    //读取配置文件
    int ret = ReadConfXML();
    if (ret == FALSE) {
        assert(!"ReadConfXML error!");
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief CommProc::ReadConfXML
 * @return
 */
int GloFunc::ReadConfXML()
{
    char filePath[COMMON_STRLEN_512];
    UtilFunc::GetApplicationDirPath(filePath);
    strcat_s(filePath, COMMON_STRLEN_512, "/config/comm_config.xml");

    QFile file(QString::fromLocal8Bit(filePath));
    if (!file.open(QIODevice::ReadOnly | QFile::Text))
    {
        file.close();
        return FALSE;
    }

    QDomDocument doc;
    if (!doc.setContent(&file))
    {
        qDebug()<<"setcontent error..." ;
        file.close();
        return FALSE;
    }
    file.close();

    QDomElement root = doc.documentElement();
    if (root.tagName() != "CONFIG")
    {
        qDebug()<<"file error";
        return FALSE;
    }

    QDomNode node = root.firstChild();
    while(!node.isNull()) {
        int ret = 0;
        QDomElement nodeElement = node.toElement();
        if (nodeElement.tagName() == "SERVER") {
            ret = ReadConf_Netsrv(nodeElement);
            if (ret == FALSE) {
                assert(!"ReadConf_Netsrv error!");
            }
        }
        else if (nodeElement.tagName() == "CLIENT") {
            ret = ReadConf_Client(nodeElement);
            if (ret == FALSE) {
                assert(!"ReadConf_Client error!");
            }
        }
        else {


        }

        node = node.nextSibling();
    }

    return TRUE;
}

int GloFunc::ReadConf_Netsrv(QDomElement &netsrv)
{
    CLIENT_COMM *comm = &glo_comm;
    if (comm == NULL) {
        return FALSE;
    }

    sprintf(comm->ipAddr, "%s", netsrv.attribute("ipAddr").toLocal8Bit().constData());
    if (strcmp(comm->ipAddr, "") == NULL) {
        return FALSE;
    }

    comm->port = netsrv.attribute("port").toInt();
    if (comm->port <= 0) {
        return FALSE;
    }

    comm->heartCheckInter = netsrv.attribute("heartCheckInter").toInt();
    if (comm->heartCheckInter <= 0) {
        return FALSE;
    }

    comm->heartSendTimeLimit = netsrv.attribute("heartSendTimeLimit").toInt();
    if (comm->heartSendTimeLimit <= 0) {
        return FALSE;
    }

    comm->heartRecvTimeLimit = netsrv.attribute("heartRecvTimeLimit").toInt();
    if (comm->heartRecvTimeLimit <= 0) {
        return FALSE;
    }

    return TRUE;
}

int GloFunc::ReadConf_Client(QDomElement &client)
{
    CLIENT_COMM *comm = &glo_comm;
    if (comm == NULL) {
        return FALSE;
    }

    comm->uuid = client.attribute("uuid").toInt();
    if (comm->uuid < 0) {
        return FALSE;
    }

    comm->exeId = client.attribute("exeId").toInt();
    if (comm->exeId < 0) {
        return FALSE;
    }

    return TRUE;
}

