
#include "pubfunc.h"

extern NETSRV_CLIDATA  glo_clients[NETSRV_CLI_NUM];
extern NETSRV_SRVDATA  glo_servers;

int PubFunc::ReadConfXML()
{
    QString filePath = QCoreApplication::applicationDirPath();
    filePath += "/config/ntconfig.xml";
    QFile file(filePath);
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

    int cliId = 0, ret = 0;
    while(!node.isNull()) {

        QDomElement nodeElement = node.toElement();
        if (nodeElement.tagName() == "CLIENT") {
            ret = ReadConf_Client(nodeElement, cliId);
            cliId++;
        }
        else if (nodeElement.tagName() == "SERVER") {
            ret = ReadConf_Server(nodeElement);
        }
        else {

        }

        node = node.nextSibling();
    }

    return TRUE;
}

int PubFunc::ReadConf_Client(QDomElement &client, int cliId)
{
    if (cliId < 0 || cliId >= NETSRV_CLI_NUM) {
        return FALSE;
    }

    NETSRV_CLIDATA *cliData = &glo_clients[cliId];
    if (cliData == NULL) {
        return FALSE;
    }

    cliData->enableFlag = client.attribute("enableFlag").toInt();
    cliData->uuid       = client.attribute("uuid").toInt();

    //COMM
    QDomNodeList nodeList = client.elementsByTagName("COMM");
    if (nodeList.size() <= 0) {
        return FALSE;
    }

    for (int i = 0; i < nodeList.size(); i++) {
        if (i >= NETSRV_CLICOMM_NUM) {
            continue;
        }

        QDomNode commNode = nodeList.at(i);
        QDomElement comm  = commNode.toElement();

        cliData->commData[i].enableFlag = comm.attribute("enableFlag").toInt();
        sprintf(cliData->commData[i].ipAddr, "%s", comm.attribute("ipAddr").toLocal8Bit().data());
    }

    return TRUE;
}

/**
 * @brief PubFunc::ReadConf_Server
 * @param server
 * @return
 */
int PubFunc::ReadConf_Server(QDomElement &server)
{
    NETSRV_SRVDATA *srvData = &glo_servers;
    if (srvData == NULL) {
        return FALSE;
    }

    srvData->listenPort    = server.attribute("listenPort").toInt();
    if (srvData->listenPort <= 0) {
        return FALSE;
    }

    srvData->saveInter     = server.attribute("saveInter").toInt();
    if (srvData->saveInter <= 0) {
        return FALSE;
    }

    srvData->saveFileInter = server.attribute("saveFileInter").toInt();
    if (srvData->saveFileInter <= 0) {
        return FALSE;
    }

    srvData->playInter     = server.attribute("playInter").toInt();
    if (srvData->playInter <= 0) {
        return FALSE;
    }

    srvData->playRate      = server.attribute("playRate").toInt();
    if (srvData->playRate <= 0) {
        return FALSE;
    }

    srvData->heartbeatInter= server.attribute("heartbeatInter").toInt();
    if (srvData->heartbeatInter <= 0) {
        return FALSE;
    }

    srvData->heartbeatTimeout= server.attribute("heartbeatTimeout").toInt();
    if (srvData->heartbeatTimeout <= 0) {
        return FALSE;
    }

    return TRUE;
}
