
#include "../common/scommunicator_def.h"
#include "../../../../utils/utils.h"
#include "pubfunc.h"

extern NETSRV_SRVDATA  glo_server;
extern NETSRV_CLIDATA  glo_clients[COMM_CLI_NUM];
extern char m_logfilePath[COMMON_STRLEN_512];

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
    UtilFunc::GetApplicationDirPath(m_logfilePath);
    strcat_s(m_logfilePath, COMMON_STRLEN_512, SYSTEM_PATH_LOG);

    memset(&glo_server,  0, sizeof(NETSRV_SRVDATA));
    memset(&glo_clients, 0, sizeof(NETSRV_CLIDATA)*COMM_CLI_NUM);

    //test 固定启用房间号为0的房间
    glo_server.roomData[0].useFlag = TRUE;

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
    strcat_s(filePath, COMMON_STRLEN_512, "/config/netserver_config.xml");

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
            if (ret != TRUE) {
                return FALSE;
            }
            cliId++;
        }
        else if (nodeElement.tagName() == "SERVER") {
            ret = ReadConf_Server(nodeElement);
            if (ret != TRUE) {
                return FALSE;
            }
        }
        else {

        }

        node = node.nextSibling();
    }

    return TRUE;
}

int GloFunc::ReadConf_Client(QDomElement &client, int cliId)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM) {
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
        if (i >= COMM_CLICOMM_NUM) {
            continue;
        }

        QDomNode commNode = nodeList.at(i);
        QDomElement comm  = commNode.toElement();

        sprintf(cliData->commData[i].ipAddr, "%s", comm.attribute("ipAddr").toLocal8Bit().data());
    }

    return TRUE;
}

int GloFunc::ReadConf_Server(QDomElement &server)
{
    NETSRV_SRVDATA *srvData = &glo_server;
    if (srvData == NULL) {
        return FALSE;
    }

    srvData->uuid    = server.attribute("uuid").toInt();
    if (srvData->uuid < 0) {
        return FALSE;
    }

    srvData->exeId    = server.attribute("exeId").toInt();
    if (srvData->exeId < 0) {
        return FALSE;
    }

    srvData->listenPort    = server.attribute("listenPort").toInt();
    if (srvData->listenPort <= 0) {
        return FALSE;
    }

    srvData->serviceProcInter     = server.attribute("serviceProcInter").toInt();
    if (srvData->serviceProcInter <= 0) {
        return FALSE;
    }

    srvData->saveFileInter = server.attribute("saveFileInter").toInt();
    if (srvData->saveFileInter <= 0) {
        return FALSE;
    }

    srvData->playRate      = server.attribute("playRate").toInt();
    if (srvData->playRate <= 0) {
        return FALSE;
    }

    srvData->heartCheckInter= server.attribute("heartCheckInter").toInt();
    if (srvData->heartCheckInter <= 0) {
        return FALSE;
    }

    srvData->heartRecvTimeLimit= server.attribute("heartRecvTimeLimit").toInt();
    if (srvData->heartRecvTimeLimit <= 0) {
        return FALSE;
    }

    srvData->userThreadFlag= server.attribute("userThreadFlag").toInt();
    if (srvData->userThreadFlag < 0) {
        return FALSE;
    }

    srvData->userThreadInter= server.attribute("userThreadInter").toInt();
    if (srvData->userThreadInter <= 0) {
        return FALSE;
    }

    srvData->forwardingMode= server.attribute("forwardingMode").toInt();
    if (srvData->forwardingMode < 0) {
        return FALSE;
    }

    return TRUE;
}
