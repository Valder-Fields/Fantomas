

//#include <QDateTime> //因与调用方的Qt版本冲突暂时弃用
#include <QTime>

#include "../../../../utils/utils.h"

#include "../../../commsocket/commsocket.h"
#include "../../../protocol/protocol.h"

#include "../pubfunc/pubfunc.h"
#include "scommmanager.h"

extern int glo_liveflag;
extern int glo_threadnum;

extern NETSRV_SRVDATA  glo_server;
extern NETSRV_CLIDATA  glo_clients[COMM_CLI_NUM];

extern CommUdpSocket *commUdpSocket;

extern char m_logfilePath[COMMON_STRLEN_512];

SCommManager::SCommManager()
{
    m_protocol = NULL;
}

SCommManager::~SCommManager()
{
    Quit();
}

int SCommManager::Init(int protoType)
{
    //配置文件
    int ret = GloFunc::InitGloFunc();

    //初始化协议对象
    switch (protoType) {
    case COMM_PROTOCOL_SimpleTlv:
    {
        m_protocol = new ProtoSimpleTlv();
    }
        break;

    default:
        ret = FALSE;
        break;
    }

    return ret;
}

int SCommManager::Quit()
{
    //关闭socket
    CloseAllCliSocket();

    if (m_protocol) {
        delete m_protocol;
        m_protocol = NULL;
    }

    return TRUE;
}

void SCommManager::AcceptProc()
{
    int  acceptCliSock = (int)INVALID_HANDLE_VALUE, cliPort;
    char cliIpAddr[COMMON_STRLEN_32];

    if (glo_server.listenPort <= 0) {
        return;
    }

    //监听
    if (glo_server.listenSocket <= 0) {
        glo_server.listenSocket = CommTcpSocket::ListenPort(glo_server.listenPort);
        if (glo_server.listenSocket <= 0) {
            qDebug()<<"ListenPort error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        }
        else {
            qDebug()<<"ListenPort success!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        }
    }

    //对客户端所有程序accept
    acceptCliSock = CommTcpSocket::Accept(glo_server.listenSocket, cliIpAddr, &cliPort);

    if (acceptCliSock > 0) {
        //2.1判断是否为客户端
        for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
            if (glo_clients[cliId].enableFlag == FALSE) {
                continue;
            }

            for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
                //找一个未连接的通道.跳过已连接的通道.
                if (glo_clients[cliId].commData[commId].connFlag == TRUE ||
                    glo_clients[cliId].commData[commId].socket > 0) {
                    continue;
                }

                //新连接的IP与新通道配置的IP一致,才进行分配
                if (strcmp(glo_clients[cliId].commData[commId].ipAddr, cliIpAddr) == NULL) {
                    qDebug()<<"connect IP:"<<cliIpAddr<<"  socket:"<<acceptCliSock<<QTime::currentTime();

                    //1.更新通讯参数
                    glo_clients[cliId].commData[commId].socket = acceptCliSock;
                    glo_clients[cliId].commData[commId].connPort = cliPort;
                    glo_clients[cliId].commData[commId].connTime_ms = UtilFunc::GetSysTimeMS();

                    glo_clients[cliId].commData[commId].heartRecvWaitCount = 0; //清空心跳计数
                    SetCommConnFlag(cliId, commId, TRUE); //设置连接标记为TRUE

                    //向模块外发出连接信息
                    CommonMessageCode msg;
                    msg.code = COMM_Code::ConnectSuccess;
                    CommonTimeInfo t;
                    UtilFunc::GetCurTimeInfo(t, glo_clients[cliId].commData[commId].connTime_ms);
                    sprintf(msg.content, "连接建立. IP:%s socket:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                            cliIpAddr, acceptCliSock, t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
                    MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);
                    break;
                }
            }
        }
    }


}

/**
 * @brief SCommManager::ReadProc 通道读
 */
void SCommManager::ReadProc()
{
    //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int64_t btime = UtilFunc::GetSysTimeMS();
    if (0) qDebug()<<"+++++++++++++++++"<<QTime::currentTime();
    int ret, rxLen = 0;
    char readBuf[COMM_SINGLE_READ_SIZE];

    //读取客户端
    for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
        if (glo_clients[cliId].enableFlag == FALSE) {
            continue;
        }

        for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
            if (glo_clients[cliId].commData[commId].connFlag == FALSE) {
                continue;
            }

            //过滤未连接通道
            if (glo_clients[cliId].commData[commId].connFlag == FALSE ||
                glo_clients[cliId].commData[commId].socket < 0) {
                continue;
            }

            //读取客户端发送的数据
            ret = CommTcpSocket::Read(glo_clients[cliId].commData[commId].socket, readBuf, COMM_SINGLE_READ_SIZE);
            if (ret >= 0) {
                rxLen = ret;
                if (0) qDebug()<<"~~~~~~~~~~~~~~~Read from:"<<glo_clients[cliId].commData[commId].ipAddr<<QTime::currentTime();
            }
            else {
                //
                rxLen = 0;
            }

            if (rxLen > 0) {
                PutRxbuf(cliId, commId, (unsigned char *)readBuf, rxLen);

                int64_t etime = UtilFunc::GetSysTimeMS();
                int64_t durTime = etime-btime;
                if ((durTime)>=0) {
                    if (0) {
                        if (rxLen > 0) qDebug()<<"+++++++++++++++read durTime:"<<durTime<<"size:"<<rxLen<<QTime::currentTime()<<" IP:"<<glo_clients[cliId].commData[commId].ipAddr;
                    }
                }

                glo_server.runData.recvAllFlow += rxLen;
            }
        }
    }

    //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //qint64 durTime = etime-btime;
    int64_t etime = UtilFunc::GetSysTimeMS();
    int64_t durTime = etime-btime;
    if ((durTime)>1) {
        if (0) qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!read time:"<<durTime;
    }
}

void SCommManager::ForwardingProc()
{
    //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int64_t btime = UtilFunc::GetSysTimeMS();

    int ret = 0, err = 0, txLen = 0;
    char val;
    //向客户端单独发送(高优先级,TCP通道)
    for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
        if (glo_clients[cliId].enableFlag == FALSE) {
            continue;
        }

        for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
            if (glo_clients[cliId].commData[commId].connFlag == FALSE) {
                continue;
            }

            //向对端单独发送(高优先级,TCP通道)
            COMM_CHANNEL_TXBUF *txbuf = GetTxBufPtr(cliId, commId);

            txbuf->onceBufLen = 0; //notice
            while(txbuf->tail != txbuf->head) {
                val = txbuf->buf[txbuf->head];
                txbuf->onceBuf[txbuf->onceBufLen] = val;
                txbuf->head = (txbuf->head + 1) % COMM_TXBUF_LEN;
                txbuf->onceBufLen++;
            }

            if (txbuf->onceBufLen > 0) {
                ret = CommTcpSocket::Write(glo_clients[cliId].commData[commId].socket, (char *)txbuf->onceBuf, txbuf->onceBufLen);
                if (ret > 0) {
                    txLen = ret;
                }
                else {
                    //通道重启流程
                    err = GetLastError();
                    qDebug()<<"ForwardingProc err:"<<err<<QTime::currentTime();
                }

                if (txLen > 0) {
                    glo_server.runData.sendAllFlow += ret;
                }

                //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
                //qint64 durTime = etime-btime;
                int64_t etime = UtilFunc::GetSysTimeMS();
                int64_t durTime = etime-btime;

                if (durTime >= 0) {
                    if (0) {
                        if (ret > 0) qDebug()<<"---------------write durTime:"<<durTime<<"size:"<<ret<<QTime::currentTime()<<" IP:"<<glo_clients[cliId].commData[commId].ipAddr<<"socket:"<<glo_clients[cliId].commData[commId].socket;
                    }
                }
            }
        }
    }

    //2.服务器转发(TCP/UDP方式)
    //if (1) qDebug()<<"ForwardingProc server forw..............1";
    for (int roomId = 0; roomId < COMM_SRVROOM_NUM; roomId++) {
        if (glo_server.roomData[roomId].useFlag == FALSE) {
            continue;
        }

        SRV_COMM_CHANNEL_TXBUF *srvTxbuf = GetSrvTxBufPtrByRoomId(roomId);
        if (srvTxbuf == NULL) {
            //依据判例一,此处应注释掉assert
            //assert(!"GetSrvTxBufPtrByRoomId error!");
            return;
        }

        srvTxbuf->onceBufLen = 0; //notice

        while(srvTxbuf->tail != srvTxbuf->head) {
            srvTxbuf->onceBuf[srvTxbuf->onceBufLen] = srvTxbuf->buf[srvTxbuf->head];
            srvTxbuf->head = (srvTxbuf->head + 1) % COMM_SRVTXBUF_LEN;
            srvTxbuf->onceBufLen++;
            if (srvTxbuf->onceBufLen >= COMM_SENDDING_LENTH) {
                char fileName[256];
                QDateTime date;
                date.setSecsSinceEpoch(glo_server.runData.startTime);
                sprintf(fileName, "NetServer_%s.log", date.toString("yyyy_MM_dd_hh_mm_ss").toLocal8Bit().constData());
                strcat_s(m_logfilePath, COMMON_STRLEN_512, fileName);

                QDateTime curDateTime = QDateTime::currentDateTime();
                char text[256];
                sprintf(text, "%s%d %04d-%02d-%02d %02d:%02d:%02d \n", "ForwardingProc: srvTxbuf->onceBufLen >= COMM_SENDDING_LENTH! bufLen:", \
                        srvTxbuf->onceBufLen, \
                        curDateTime.date().year(), curDateTime.date().month(), curDateTime.date().day(), curDateTime.time().hour(), curDateTime.time().minute(), curDateTime.time().second());
                UtilFunc::SaveFile(m_logfilePath, text, (int)strlen(text));
                assert(!"ForwardingProc...1");
                break;
            }
        }

        if (srvTxbuf->onceBufLen > 0) {
            if (srvTxbuf->onceBufLen >= COMM_SENDDING_LENTH) {
                qDebug()<<"!!!!!!!!!!!!!!!!!!!!!! beyond onceBufLen limit:"<<COMM_SENDDING_LENTH<<" but still send!";
            }
            for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
                if (glo_clients[cliId].enableFlag == FALSE) {
                    continue;
                }

                //不是一个房间的不转发
                if (glo_clients[cliId].roomId != roomId) {
                    continue;
                }

                for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
                    //过滤未连接客户端
                    if (glo_clients[cliId].commData[commId].connFlag == FALSE ||
                        glo_clients[cliId].commData[commId].socket < 0) {
                        continue;
                    }

                    ret = CommTcpSocket::Write(glo_clients[cliId].commData[commId].socket, srvTxbuf->onceBuf, srvTxbuf->onceBufLen);
                    if (ret <= 0) {
                        err = GetLastError();
                        qDebug()<<"ForwardingProc-cliId:"<<cliId<<"commId:"<<commId<<"exeId:"<<glo_clients[cliId].commData[commId].exeId<<"err:"<<err<<QTime::currentTime();
                    }
                    else {
                        glo_server.runData.sendAllFlow += ret;
                    }

                    //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
                    //qint64 durTime = etime-btime;
                    int64_t etime = UtilFunc::GetSysTimeMS();
                    int64_t durTime = etime-btime;
                    if (0) {
                        if (durTime >= 0) {
                            qDebug()<<"---------------forw durTime:"<<durTime<<"size:"<<ret<<QTime::currentTime()<<" IP:"<<glo_clients[cliId].commData[commId].ipAddr<<"socket:"<<glo_clients[cliId].commData[commId].socket;
                        }
                    }
                }
            }
        }
    }
}

NETSRV_SRVDATA * SCommManager::GetServerData()
{
    return &glo_server;
}

NETSRV_CLIDATA * SCommManager::GetClientData(int cliId)
{
    return &glo_clients[cliId];
}

CommUdpSocket * SCommManager::GetUdpSocketHandler()
{
    return commUdpSocket;
}

COMM_CHANNEL_RXBUF * SCommManager::GetRxBufPtr(int cliId, int commId)
{
    return &glo_clients[cliId].commData[commId].rxbuf;
}

COMM_CHANNEL_TXBUF * SCommManager::GetTxBufPtr(int cliId, int commId)
{
    return &glo_clients[cliId].commData[commId].txbuf;
}

void SCommManager::GetRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char *val)
{
    *val = rxbuf->buf[rxbuf->head];
    rxbuf->head = (rxbuf->head + 1) % COMM_RXBUF_LEN;
}

void SCommManager::PutRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char val)
{
    rxbuf->buf[rxbuf->tail] = val;
    rxbuf->tail = (rxbuf->tail+1) % COMM_RXBUF_LEN;
}

void SCommManager::PutTxbufVal(COMM_CHANNEL_TXBUF *txbuf, unsigned char val)
{
    txbuf->buf[txbuf->tail] = val;
    txbuf->tail = (txbuf->tail + 1) % COMM_TXBUF_LEN;
}

int SCommManager::GetRxbuf(int cliId, int commId, unsigned char *buffer, int bufferLen)
{
    if (buffer == NULL || bufferLen < 0) return FALSE;

    COMM_CHANNEL_RXBUF *rxbuf = GetRxBufPtr(cliId, commId);
    if (rxbuf == NULL) {
        assert(!"GetRxBufPtr error!");
        return FALSE;
    }

    if ((rxbuf->head+bufferLen) < COMM_RXBUF_LEN) {
        memcpy(buffer, &rxbuf->buf[rxbuf->head], bufferLen);
    }
    else {
        for (int i = 0; i < bufferLen; i++) {
            buffer[i] = rxbuf->buf[(rxbuf->head+i)%COMM_RXBUF_LEN];
        }
    }

    return TRUE;
}

int SCommManager::PutRxbuf(int cliId, int commId, unsigned char *buffer, int bufferLen)
{
    if (bufferLen <= 0 || buffer == NULL) return FALSE;

    COMM_CHANNEL_RXBUF *rxbuf = GetRxBufPtr(cliId, commId);
    if (rxbuf == NULL) {
        assert(!"GetRxBufPtr error!");
        return FALSE;
    }

    if ((rxbuf->tail+bufferLen)<COMM_RXBUF_LEN) {
        memcpy(&rxbuf->buf[rxbuf->tail], buffer, bufferLen);
        rxbuf->tail += bufferLen;
    }
    else {
        for (int i = 0; i < bufferLen; i++) {
            PutRxbufVal(rxbuf, buffer[i]);
        }
    }

    return TRUE;
}

int SCommManager::PutTxBuf(int cliId, int commId, unsigned char *buffer, int bufferLen)
{
     if (bufferLen <= 0 || buffer == NULL) return FALSE;

     COMM_CHANNEL_TXBUF *txbuf = GetTxBufPtr(cliId, commId);
     if ((txbuf->tail+bufferLen)<COMM_TXBUF_LEN) {
         memcpy(&txbuf->buf[txbuf->tail], buffer, bufferLen);
         txbuf->tail += bufferLen;
     }
     else {
         for (int i = 0; i < bufferLen; i++) {
             PutTxbufVal(txbuf, buffer[i]);
         }
     }

     return TRUE;
}

SRV_COMM_CHANNEL_TXBUF * SCommManager::GetSrvTxBufPtr(int cliId)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM) {
        return NULL;
    }

    int roomId = glo_clients[cliId].roomId;
    if (roomId < 0 || roomId >= COMM_SRVROOM_NUM) {
        return NULL;
    }

    return &glo_server.roomData[roomId].commData.txbuf;
}

SRV_COMM_CHANNEL_TXBUF * SCommManager::GetSrvTxBufPtrByRoomId(int roomId)
{
    if (roomId < 0 || roomId >= COMM_SRVROOM_NUM) {
        return NULL;
    }

    return &glo_server.roomData[roomId].commData.txbuf;
}

void SCommManager::PutSrvTxbufVal(SRV_COMM_CHANNEL_TXBUF *txbuf, unsigned char val)
{
    txbuf->buf[txbuf->tail] = val;
    txbuf->tail = (txbuf->tail + 1) % COMM_SRVTXBUF_LEN;
}

int SCommManager::PutSrvForwardingBuf(int cliId, unsigned char *buffer, int frameLen)
{
    if (buffer == NULL) {
        return FALSE;
    }

    SRV_COMM_CHANNEL_TXBUF *srvtxbuf = GetSrvTxBufPtr(cliId);
    if (srvtxbuf == NULL) {
        //依据判例一,此处应注释掉assert
        //assert(!"GetSrvTxBufPtr error!");
        return FALSE;
    }

    if ((srvtxbuf->tail+frameLen) < COMM_SRVTXBUF_LEN) {
        memcpy(&srvtxbuf->buf[srvtxbuf->tail], buffer, frameLen);
        srvtxbuf->tail += frameLen;
    }
    else {
        for (int i = 0; i < frameLen; i++) {
            PutSrvTxbufVal(srvtxbuf, buffer[i]);
        }
    }

    return TRUE;
}

int SCommManager::PutSrvForwardingBufByRoomId(int roomId, unsigned char *buffer, int frameLen)
{
    if (buffer == NULL || roomId < 0 || roomId >= COMM_SRVROOM_NUM) {
        return FALSE;
    }

    SRV_COMM_CHANNEL_TXBUF *srvtxbuf = &glo_server.roomData[roomId].commData.txbuf;

    if ((srvtxbuf->tail+frameLen) < COMM_SRVTXBUF_LEN) {
        memcpy(&srvtxbuf->buf[srvtxbuf->tail], buffer, frameLen);
        srvtxbuf->tail += frameLen;
    }
    else {
        for (int i = 0; i < frameLen; i++) {
            PutSrvTxbufVal(srvtxbuf, buffer[i]);
        }
    }

    return TRUE;
}

int SCommManager::SendFrame(int cliId, int commId, int type, int length, unsigned char *value)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) return FALSE;

    if (value == NULL) {
        return FALSE;
    }

    if (length < 0 || length > COMM_FRAME_MAXNUM) {
        return FALSE;
    }

    int ret = FALSE;

    //1.由协议完成组帧
    int frameLen = 0;
    unsigned char frame[COMM_FRAME_MAXNUM];

    if (m_protocol) {
        ret = m_protocol->MakeAWholeFrame(type, value, length, frame, frameLen);
    }

    if (ret != TRUE) {
        return ret;
    }

    //2.由manager放入发送缓存
    ret = PutTxBuf(cliId, commId, frame, frameLen);

    //3.发送帧数+1
    glo_server.runData.sendAllFrame++;

    return ret;
}

int SCommManager::GetAWholeFrame(int cliId, int commId, int &type, unsigned char *frame, int &frameLen)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) return FALSE;

    if (m_protocol == NULL) {
        return FALSE;
    }

    COMM_CHANNEL_RXBUF *rxbuf = GetRxBufPtr(cliId, commId);
    if (rxbuf == NULL) {
        return FALSE;
    }

    int ret = FALSE;

    ret = m_protocol->GetAWholeFrame(rxbuf, type, frame, frameLen);

    return ret;
}

int SCommManager::GetValueByFrame(unsigned char *frame, int frameLen, unsigned char **value, int &valueLen)
{
    if (m_protocol == NULL) {
        return FALSE;
    }

    return m_protocol->GetValueByFrame(frame, frameLen, value, valueLen);
}

int SCommManager::MakeAWholeFrame(int type, unsigned char *value, int valueLen, unsigned char *frame, int &frameLen)
{
    return m_protocol->MakeAWholeFrame(type, value, valueLen, frame, frameLen);
}

int SCommManager::isHeartSendTimeout(int cliId, int commId)
{
    (void)cliId;
    (void)commId;
    /*if (glo_clients[cliId].commData[commId].heartSendWaitCount*glo_server.heartCheckInter >= glo_server.heartSendTimeLimit) {
        return TRUE;
    }
    */
    return FALSE;
}

int SCommManager::isHeartRecvTimeout(int cliId, int commId)
{
    if (glo_clients[cliId].commData[commId].heartRecvWaitCount*glo_server.heartCheckInter > glo_server.heartRecvTimeLimit) {
        return TRUE;
    }

    return FALSE;
}

int SCommManager::SetCommConnFlag(int cliId, int commId, int flag)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) {
        return FALSE;
    }

    glo_clients[cliId].commData[commId].connFlag = flag;

    return TRUE;
}

int SCommManager::isConnected(int cliId, int commId)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) {
        return FALSE;
    }

    return glo_clients[cliId].commData[commId].connFlag;
}

void SCommManager::ClearHeartRecvWaitCount(int cliId, int commId)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) {
        return;
    }

    glo_clients[cliId].commData[commId].heartRecvWaitCount = 0;
}

void SCommManager::CommBrokenProc(int cliId, int commId)
{
    //关闭socket
    CommBase::CloseSocket(glo_clients[cliId].commData[commId].socket);

    //向模块外发出连接信息
    CommonMessageCode msg;
    msg.code = COMM_Code::ConnectBreak;
    CommonTimeInfo t;
    UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
    sprintf(msg.content, "连接断开. IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
            GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
            cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
            t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
    MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);
    //重设通道临时数据(非固定数据)
    SetCommConnFlag(cliId, commId, FALSE); //设置断线标记
    glo_clients[cliId].commData[commId].socket = 0;
    glo_clients[cliId].commData[commId].connTime_ms = 0;
    glo_clients[cliId].commData[commId].lockFlag = 0;
    glo_clients[cliId].commData[commId].exeId = 0;
    glo_clients[cliId].commData[commId].connPort = 0;
    glo_clients[cliId].commData[commId].heartSendWaitCount = 0;
    glo_clients[cliId].commData[commId].heartRecvWaitCount = 0;
    memset(&glo_clients[cliId].commData[commId].rxbuf, 0, sizeof(COMM_CHANNEL_RXBUF));
    memset(&glo_clients[cliId].commData[commId].txbuf, 0, sizeof(COMM_CHANNEL_TXBUF));

    //todo 如果为SysClient则需向SysController发出客户端断线消息

    if (1) qDebug()<<"CommBrokenProc!!!!!!!!!!!!!!!!!! IP:"<<GetClientData(cliId)->commData[commId].ipAddr
                   <<"uuid"<<GetClientData(cliId)->uuid
                   <<"exeId:"<<GetClientData(cliId)->commData[commId].exeId<<QTime::currentTime();
}

void SCommManager::CloseAllCliSocket()
{
    for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
        if (glo_clients[cliId].enableFlag == FALSE) {
            continue;
        }

        for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
            if (glo_clients[cliId].commData[commId].connFlag == FALSE) {
                continue;
            }

            CommBase::CloseSocket(glo_clients[cliId].commData[commId].socket);

            //重设通道临时数据(非固定数据)
            SetCommConnFlag(cliId, commId, FALSE); //设置断线标记
            glo_clients[cliId].commData[commId].socket = 0;
            glo_clients[cliId].commData[commId].connTime_ms = 0;
        }
    }
}

/**
 * @brief srv_acceptproc    客户端连接监听线程
 * @param pParam
 * @return
 */
UINT srv_acceptproc(LPVOID pParam)
{
    SCommManager *commproc = (SCommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    unsigned int count = 0;
    while (glo_liveflag) {

        commproc->AcceptProc();

        count ++;

        Sleep(COMM_SRVACCEPTPROC_INTER);
    }

    glo_threadnum --;
    return 0;
}


/**
 * @brief srv_rwproc    读线程
 * @param pParam
 * @return
 */
UINT srv_readproc(LPVOID pParam)
{
    SCommManager *commproc = (SCommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    while (glo_liveflag) {

        commproc->ReadProc();

        Sleep(COMM_SRVREADPROC_INTER);
    }

    glo_threadnum --;
    return 0;
}

/**
 * @brief srv_forwardingproc 转发
 * @param pParam
 * @return
 */
UINT srv_forwardingproc(LPVOID pParam)
{
    SCommManager *commproc = (SCommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    while (glo_liveflag) {

        commproc->ForwardingProc();

        Sleep(COMM_SRVFORWARDING_INTER);
    }

    glo_threadnum --;
    return 0;
}

/**
 * @brief srv_dispatchproc    解析线程
 * @param pParam
 * @return
 */
UINT srv_dispatchproc(LPVOID pParam)
{
    SCommManager *commproc = (SCommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    while (glo_liveflag) {

        commproc->DispatchProc();

        Sleep(COMM_SRVDISPATCHPROC_INTER);
    }

    glo_threadnum --;
    return 0;
}

/**
 * @brief srv_serviceproc    服务线程
 * @param pParam
 * @return
 */
UINT srv_serviceproc(LPVOID pParam)
{
    SCommManager *commproc = (SCommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    static int serviceProcInter = glo_server.serviceProcInter;

    while (glo_liveflag) {

        commproc->ServiceProc();

        Sleep(serviceProcInter);
    }

    glo_threadnum --;
    return 0;
}

/**
 * @brief srv_userproc    用户线程
 * @param pParam
 * @return
 */
UINT srv_userproc(LPVOID pParam)
{
    SCommManager *commproc = (SCommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    static int userThreadInter = glo_server.userThreadInter;

    while (glo_liveflag) {

        commproc->UserProc();

        Sleep(userThreadInter);
    }

    glo_threadnum --;
    return 0;
}
