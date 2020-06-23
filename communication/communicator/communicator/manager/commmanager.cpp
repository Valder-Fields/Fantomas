
//#include <QDateTime> //因与调用方的Qt版本冲突暂时弃用
#include <QTime>

#include "../../../../utils/utils.h"
#include "../pubfunc/pubfunc.h"

#include "../commsocket/commsocket.h"
#include "../protocol/protocol.h"

#include "commmanager.h"

extern int glo_liveflag;
extern int glo_threadnum;

extern CLIENT_COMM  glo_comm;

CommManager::CommManager()
{
    m_protocol = NULL;
}

CommManager::~CommManager()
{
    Quit();
}

int CommManager::Init(int protoType)
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

int CommManager::Quit()
{
    //关闭socket
    CommBase::CloseSocket(glo_comm.socket);

    //重设通道临时数据(非固定数据)
    SetCommConnFlag(FALSE); //设置断线标记
    glo_comm.socket = 0;
    glo_comm.connTime_ms = 0;

    if (m_protocol) {
        delete m_protocol;
        m_protocol = NULL;
    }

    return TRUE;
}

int CommManager::GetUuid()
{
    return glo_comm.uuid;
}

int CommManager::GetExeId()
{
    return glo_comm.exeId;
}

/**
 * @brief CommManager::ReadProc 通道读
 */
void CommManager::ReadProc()
{
    CheckConnectProc();

    //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int64_t btime = UtilFunc::GetSysTimeMS();
    if (0) qDebug()<<"+++++++++++++++++"<<QTime::currentTime();
    int ret, rxLen = 0;
    char readBuf[COMM_SINGLE_READ_SIZE];

    //读取通道
    //过滤未连接通道
    if (glo_comm.connFlag == FALSE ||
        glo_comm.socket < 0) {
        return;
    }

    //读取对端发送的数据
    ret = CommTcpSocket::Read(glo_comm.socket, readBuf, COMM_SINGLE_READ_SIZE);
    if (ret >= 0) {
        rxLen = ret;
        if (0) qDebug()<<"~~~~~~~~~~~~~~~Read from:"<<glo_comm.ipAddr<<QTime::currentTime();
    }
    else {
        //
        rxLen = 0;
    }

    if (rxLen > 0) {
        PutRxbuf((unsigned char *)readBuf, rxLen);

        //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        //qint64 durTime = etime-btime;
        int64_t etime = UtilFunc::GetSysTimeMS();
        int64_t durTime = etime-btime;
        if ((durTime)>=0) {
            if (1) {
                if (rxLen > 0) qDebug()<<"+++++++++++++++read durTime:"<<durTime<<"size:"<<rxLen<<QTime::currentTime()<<" IP:"<<glo_comm.ipAddr;
            }
        }

        glo_comm.runData.recvAllFlow += rxLen;
    }

    //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //qint64 durTime = etime-btime;
    int64_t etime = UtilFunc::GetSysTimeMS();
    int64_t durTime = etime-btime;
    if ((durTime)>1) {
        if (0) qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!read time:"<<durTime;
    }
}

void CommManager::ForwardingProc()
{
    CheckConnectProc();

    //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int64_t btime = UtilFunc::GetSysTimeMS();

    int ret = 0, err = 0;
    char val;

    //向对端单独发送(高优先级,TCP通道)
    COMM_CHANNEL_TXBUF *txbuf = GetTxBufPtr();

    txbuf->onceBufLen = 0; //notice
    while(txbuf->tail != txbuf->head) {
        val = txbuf->buf[txbuf->head];
        txbuf->onceBuf[txbuf->onceBufLen] = val;
        txbuf->head = (txbuf->head + 1) % COMM_TXBUF_LEN;
        txbuf->onceBufLen++;
    }

    if (txbuf->onceBufLen > 0) {
        ret = CommTcpSocket::Write(glo_comm.socket, (char *)txbuf->onceBuf, txbuf->onceBufLen);
        if (ret <= 0) {
            //通道重启流程
            err = GetLastError();
            qDebug()<<"ForwardingProc err:"<<err<<QTime::currentTime();
        }
        else {
            glo_comm.runData.sendAllFlow += ret;
        }

        //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        //qint64 durTime = etime-btime;
        int64_t etime = UtilFunc::GetSysTimeMS();
        int64_t durTime = etime-btime;

        if (durTime >= 0) {
            if (1) {
                if (ret > 0) qDebug()<<"---------------write durTime:"<<durTime<<"size:"<<ret<<QTime::currentTime()<<" IP:"<<glo_comm.ipAddr<<"socket:"<<glo_comm.socket;
            }
        }
    }
}

void CommManager::DispatchProc()
{
    //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int64_t btime = UtilFunc::GetSysTimeMS();

    //过滤未连接通道(后期加入断线重连业务时可不过滤未连接通道,防止通道突然断开后还有数据未处理)
    if (isConnected() == FALSE) {
        return;
    }

    //1.接收处理
    RxChannelProc();

    //2.发送处理
    TxChannelProc();

    glo_comm.sysData.dispatchProcLoopCount++;

    //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //qint64 durTime = etime-btime;
    int64_t etime = UtilFunc::GetSysTimeMS();
    int64_t durTime = etime-btime;
    if (durTime>1) {
        if (0) qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!DispatchProc time:"<<durTime;
    }
}

COMM_CHANNEL_RXBUF * CommManager::GetRxBufPtr()
{
    return &glo_comm.rxbuf;
}

COMM_CHANNEL_TXBUF * CommManager::GetTxBufPtr()
{
    return &glo_comm.txbuf;
}

void CommManager::GetRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char *val)
{
    *val = rxbuf->buf[rxbuf->head];
    rxbuf->head = (rxbuf->head + 1) % COMM_RXBUF_LEN;
}

void CommManager::PutRxbufVal(COMM_CHANNEL_RXBUF *rxbuf, unsigned char val)
{
    rxbuf->buf[rxbuf->tail] = val;
    rxbuf->tail = (rxbuf->tail+1) % COMM_RXBUF_LEN;
}

void CommManager::PutCliTxbufVal(COMM_CHANNEL_TXBUF *txbuf, unsigned char val)
{
    txbuf->buf[txbuf->tail] = val;
    txbuf->tail = (txbuf->tail + 1) % COMM_TXBUF_LEN;
}

int CommManager::GetRxbuf(unsigned char *buffer, int bufferLen)
{
    if (buffer == NULL || bufferLen < 0) return FALSE;

    COMM_CHANNEL_RXBUF *rxbuf = GetRxBufPtr();
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

int CommManager::PutRxbuf(unsigned char *buffer, int bufferLen)
{
    if (bufferLen <= 0 || buffer == NULL) return FALSE;

    COMM_CHANNEL_RXBUF *rxbuf = GetRxBufPtr();
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

int CommManager::PutTxBuf(unsigned char *buffer, int bufferLen)
{
     if (bufferLen <= 0 || buffer == NULL) return FALSE;

     COMM_CHANNEL_TXBUF *txbuf = GetTxBufPtr();
     if ((txbuf->tail+bufferLen)<COMM_TXBUF_LEN) {
         memcpy(&txbuf->buf[txbuf->tail], buffer, bufferLen);
         txbuf->tail += bufferLen;
     }
     else {
         for (int i = 0; i < bufferLen; i++) {
             PutCliTxbufVal(txbuf, buffer[i]);
         }
     }

     return TRUE;
}

int CommManager::SendFrame(int type, int length, unsigned char *value)
{
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
    ret = PutTxBuf(frame, frameLen);

    //3.发送帧数+1
    glo_comm.runData.sendAllFrame++;

    return ret;
}

int CommManager::GetAWholeFrame(int &type, unsigned char *frame, int &frameLen)
{
    if (m_protocol == NULL) {
        return FALSE;
    }

    COMM_CHANNEL_RXBUF *rxbuf = GetRxBufPtr();
    if (rxbuf == NULL) {
        return FALSE;
    }

    int ret = FALSE;

    ret = m_protocol->GetAWholeFrame(rxbuf, type, frame, frameLen);

    return ret;
}

int CommManager::GetValueByFrame(unsigned char *frame, int frameLen, unsigned char **value, int &valueLen)
{
    if (m_protocol == NULL) {
        return FALSE;
    }

    return m_protocol->GetValueByFrame(frame, frameLen, value, valueLen);
}

int CommManager::RxChannelProc()
{
    if (isConnected() == FALSE) {
        return FALSE;
    }

    int getFrameFlag = TRUE, ret = FALSE, type = 0, valueLen = 0;
    unsigned char *value = NULL;

    while(getFrameFlag == TRUE) {
        getFrameFlag = GetAWholeFrame(type, GetRxBufPtr()->onceFrame, GetRxBufPtr()->onceFrameLen);

        //处理帧
        if (getFrameFlag == TRUE) {
            switch (type) {
            //模块内部处理
            case COMM_FRAMETYPE_Heartbeat:
            case COMM_FRAMETYPE_LoginAck:
            case COMM_FRAMETYPE_FileSendRequest:
            case COMM_FRAMETYPE_FileSendContent:
            case COMM_FRAMETYPE_FileTransState:
            {
                ret = DealOriginFrame(type, GetRxBufPtr()->onceFrameLen, GetRxBufPtr()->onceFrame);
            }
                break;

            //模块外部处理
            default:
            {
                ret = GetValueByFrame(GetRxBufPtr()->onceFrame, GetRxBufPtr()->onceFrameLen, &value, valueLen);

                //外部处理接口
                ret = DealFrame(type, valueLen, value);
            }
                break;
            }

            //注意! head+
            //移动CLIRXBUF.head指针
            GetRxBufPtr()->head = (GetRxBufPtr()->head + GetRxBufPtr()->onceFrameLen) % COMM_RXBUF_LEN;

            glo_comm.runData.recvAllFrame++;
        }
    }

    return TRUE;
}

int CommManager::TxChannelProc()
{
    //命令业务
    CommandProc();

    //心跳机制
    int64_t curTime = UtilFunc::GetSysTimeMS();
    if ((curTime - glo_comm.lastHeartCheckTime) > glo_comm.heartCheckInter) {
        HeartbeatProc();
        glo_comm.lastHeartCheckTime = curTime;
        glo_comm.sysData.heartCheckCount++;
    }

    return TRUE;
}

void CommManager::CheckConnectProc()
{
    if (isConnected() == TRUE) {
        return;
    }

    if (glo_comm.lockFlag == FALSE) {
        //在此对连接加锁
        glo_comm.lockFlag = TRUE;

        int sockid = CommTcpSocket::ConnectChannel(glo_comm.ipAddr, glo_comm.port);
        if (sockid > 0) {
            glo_comm.heartRecvWaitCount = 0; //清空心跳接收计数

            glo_comm.socket = sockid;
            glo_comm.connTime_ms = UtilFunc::GetSysTimeMS();

            SetCommConnFlag(TRUE); //设置连接标记为TRUE

            //向服务器发送登录认证
            FRM_LoginReq v;
            v.uuid  = glo_comm.uuid;
            v.exeId = glo_comm.exeId;

            SendFrame(COMM_FRAMETYPE_LoginReq, sizeof(FRM_LoginReq), (unsigned char*)&v);

            //向模块外发出连接成功信息
            MessageCode msg;
            msg.code = COMM_Code::ConnectSuccess;
            MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);

            if (1) qDebug()<<"connect Server!!!!!!!!!!!!!!!! socket:"<<glo_comm.socket<<QTime::currentTime();
        }

        //在此对连接解锁
        glo_comm.lockFlag = FALSE;
    }
}

void CommManager::HeartbeatProc()
{
    //1.心跳发送
    if (isHeartSendTimeout() == TRUE) {
        unsigned char frame[COMMON_STRLEN_32];
        int frameLen = 0;

        Heartbeat v;
        v.state = 1;

        m_protocol->MakeAWholeFrame(COMM_FRAMETYPE_Heartbeat, (unsigned char *)&v, sizeof(v), frame, frameLen);
        PutTxBuf(frame, frameLen);

        glo_comm.heartSendWaitCount = 0;
    }

    glo_comm.heartSendWaitCount++;

    //2.心跳接收
    if (isHeartRecvTimeout() == TRUE) {    //判断超时
        if (isConnected() == TRUE) {  //判断是否已断线
            //1.执行断线流程
            CommBrokenProc();
        }
    }

    glo_comm.heartRecvWaitCount++;
}

int CommManager::isHeartSendTimeout()
{
    if (glo_comm.heartSendWaitCount*glo_comm.heartCheckInter >= glo_comm.heartSendTimeLimit) {
        return TRUE;
    }

    return FALSE;
}

int CommManager::isHeartRecvTimeout()
{
    if (glo_comm.heartRecvWaitCount*glo_comm.heartCheckInter > glo_comm.heartRecvTimeLimit) {
        return TRUE;
    }

    return FALSE;
}

int CommManager::SetCommConnFlag(int flag)
{
    glo_comm.connFlag = flag;

    return TRUE;
}

int CommManager::isConnected()
{
    return glo_comm.connFlag;
}

void CommManager::CommBrokenProc()
{
    //关闭socket
    CommBase::CloseSocket(glo_comm.socket);

    //重设通道临时数据(非固定数据)
    SetCommConnFlag(FALSE); //设置断线标记
    glo_comm.socket = 0;
    glo_comm.connTime_ms = 0;
    glo_comm.lockFlag = 0;
    glo_comm.heartSendWaitCount = 0;
    glo_comm.heartRecvWaitCount = 0;

    memset(&glo_comm.rxbuf, 0, sizeof(COMM_CHANNEL_RXBUF));
    memset(&glo_comm.txbuf, 0, sizeof(COMM_CHANNEL_TXBUF));

    //向模块外发出连接断开信息
    MessageCode msg;
    msg.code = COMM_Code::ConnectBreak;
    MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);

    if (1) qDebug()<<"CliCommBrokenProc!!!!!!!!!!!!!!!!!! IP:"<<glo_comm.ipAddr<<QTime::currentTime();
}

void CommManager::CommandProc()
{
    switch (glo_comm.cmdData.curCmd) {
    case CommandState::Idle:
    {
        //do nothing
    }
        break;
    case CommandState::FileSend:
    {
        FileSendProc();
    }
        break;

    default:
        break;
    }
}

int CommManager::FileSendProc()
{
    CMD_FILESEND *fileSend = &glo_comm.cmdData.fileSend;

    //间隔计时处理
    int64_t curTime = UtilFunc::GetSysTimeMS();
    if ((curTime-fileSend->lastSendTime) < fileSend->sendInter) {
        return FALSE;
    }
    else {
        fileSend->lastSendTime = curTime;
    }

    switch (fileSend->state) {
    case CommandState::FileSendInit:
    {
        int ret = 0;

        //获取文件总字节数
        FILE *readFp = NULL;
        ret = UtilFunc::GetFileLength(readFp, fileSend->fullFilePath, fileSend->fileLen);
        if (!ret) {
            fileSend->state = CommandState::FileSendError;
            return FALSE;
        }

        if (fileSend->fileLen > CMD_FILESEND_LENTH_LIMIT) {
            qDebug()<<"error! file is too long!!!!!!!!!!!!!!!!!!!!!";
            fileSend->state = CommandState::FileSendError;
            return FALSE;
        }

        //获取文件内容
        int64_t btime = UtilFunc::GetSysTimeMS();

        fileSend->fileBuf = (unsigned char *)malloc(fileSend->fileLen);
        if (fileSend->fileBuf == NULL) {
            assert(!"ProcFileSend malloc error!");
        }

        ret = UtilFunc::ReadFile(readFp, fileSend->fullFilePath, fileSend->fileBuf, fileSend->fileLen);
        if (!ret) {
            fileSend->state = CommandState::FileSendError;
            return FALSE;
        }

        int64_t etime = UtilFunc::GetSysTimeMS();
        int64_t durTime = etime-btime;

        qDebug()<<"malloc and read file time! durTime:"<<durTime<<QTime::currentTime();

        //发送传输文件请求帧
        FRM_FileSendReq v;
        v.destId = fileSend->destId;
        v.srcId = fileSend->srcId;
        v.fileLen = fileSend->fileLen;

        ret = UtilFunc::GetFileNameByFullFilePath(fileSend->fullFilePath, v.fileName);
        if (!ret) {
            fileSend->state = CommandState::FileSendError;
            return FALSE;
        }

        SendFrame(COMM_FRAMETYPE_FileSendRequest, sizeof(FRM_FileSendReq), (unsigned char*)&v);

        //命令状态置位
        fileSend->state = CommandState::FileSendSending;

        qDebug()<<"file send request"<<QTime::currentTime();

        //发送给通讯模块外部
        MessageCode msg;
        msg.code = COMM_Code::CMD_FileSendRequest;

        MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
    }
        break;

    case CommandState::FileSendSending:
    {
        int finishFlag = FALSE;
        int sendLen = 0;

        //剩余字节数>单次发送最大数,则发送"单次发送最大数"
        int sendIndex = fileSend->sendIndex;
        if ((fileSend->fileLen-sendIndex) > CMD_FILESEND_SINGLE_LENTH) {
            sendLen = CMD_FILESEND_SINGLE_LENTH;
        }
        else {
            //最后一帧文件内容
            sendLen = fileSend->fileLen-sendIndex;

            finishFlag = TRUE;
        }

        //sendIndex增加
        fileSend->sendIndex += sendLen;

        //发送文件内容帧
        FRM_FileSendContent v;
        v.destId = fileSend->destId;
        v.srcId  = fileSend->srcId;
        v.size   = sendLen;
        memcpy(v.content, &fileSend->fileBuf[sendIndex], sendLen);

        SendFrame(COMM_FRAMETYPE_FileSendContent, sizeof(FRM_FileSendContent), (unsigned char*)&v);

        //命令状态置位---发送已完成
        if (finishFlag) {
            fileSend->state = CommandState::FileSendFinish;
        }
        qDebug()<<"file send content"<<QTime::currentTime();

        //发送给通讯模块外部
        MessageCode msg;
        msg.code = COMM_Code::CMD_FileSendSending;

        MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
    }
        break;

    case CommandState::FileSendFinish:
    {
        //1.向对端发送文件传输结束帧
        SendFileSendState(CommandState::FileSendFinish);

        //2.复归所有数据(传输过程数据初始化)
        InitFileSendProc();
        glo_comm.cmdData.curCmd = CommandState::Idle;
        qDebug()<<"file send finish"<<QTime::currentTime();

        //发送给通讯模块外部
        MessageCode msg;
        msg.code = COMM_Code::CMD_FileSendFinish;

        MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
    }
        break;

    case CommandState::FileSendError:
    {
        //1,复归所有数据
        InitFileSendProc();
        glo_comm.cmdData.curCmd = CommandState::Idle;
        qDebug()<<"file send error"<<QTime::currentTime();

        //2.向对端发送命令执行错误帧
        SendFileSendState(CommandState::FileSendError);

        //发送给通讯模块外部
        MessageCode msg;
        msg.code = COMM_Code::CMD_FileSendError;
        MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
    }
        break;

    default:

        break;
    }

    return TRUE;
}

void CommManager::InitFileSendProc()
{
    CMD_FILESEND *fileSend = &glo_comm.cmdData.fileSend;

    fileSend->destId = 0;
    fileSend->srcId = 0;
    fileSend->overtime = 0;
    fileSend->sendInter = 0;
    memset(fileSend->fullFilePath, 0, strlen(fileSend->fullFilePath));

    fileSend->state = 0;
    fileSend->lastSendTime = 0;
    fileSend->sendIndex = 0;
    fileSend->fileLen = 0;

    if (fileSend->fileBuf) {
        free(fileSend->fileBuf);
        fileSend->fileBuf = NULL;
    }
}

int CommManager::CheckFileSendProcState(CMD_FILESEND *fileSend, int state)
{
    if (fileSend == NULL) {
        return FALSE;
    }

    if (fileSend->state == state) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void CommManager::StopFileSendProc()
{
    //1.向发送方发送接收错误帧,终止传输过程
    SendFileSendState(CommandState::FileSendError);

    //2.初始化文件接收流程
    InitFileSendProc();
}

int CommManager::SendFileSendState(int state)
{
    FRM_FileTransState v;
    v.destId = glo_comm.cmdData.fileSend.destId;
    v.srcId  = glo_comm.cmdData.fileSend.srcId;
    v.state = state;

    SendFrame(COMM_FRAMETYPE_FileTransState, sizeof(FRM_FileTransState), (unsigned char *)&v);

    return TRUE;
}

int CommManager::SendFile(int destId, int srcId, char *fullFilePath)
{
    if (fullFilePath == NULL) {
        return FALSE;
    }

    //check
    if (glo_comm.cmdData.curCmd != CommandState::Idle) {
        return FALSE;
    }

    //赋值
    glo_comm.cmdData.curCmd = CommandState::FileSend;
    CMD_FILESEND *fileSend  = &glo_comm.cmdData.fileSend;
    fileSend->destId = destId;
    fileSend->srcId  = srcId;
    fileSend->sendInter = COMM_READPROC_INTER;
    fileSend->state  = CommandState::FileSendInit;
    fileSend->lastSendTime = UtilFunc::GetSysTimeMS();
    sprintf(fileSend->fullFilePath, "%s", fullFilePath);

    //向模块外发出连接成功信息
    MessageCode msg;
    msg.code = COMM_Code::CMD_FileSendInit;
    MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);

    return TRUE;
}

int CommManager::DealOriginFrame(int type, int frameLen, unsigned char *frame)
{
    if (frame == NULL) {
        return FALSE;
    }

    int ret = FALSE;

    switch (type) {
    //登录认证
    case COMM_FRAMETYPE_LoginAck:
    {
        unsigned char *value = NULL;
        int valueLen = 0;

        ret = GetValueByFrame(frame, frameLen, &value, valueLen);

        FRM_LoginAck *ack = (FRM_LoginAck *)value;

        //向模块外发出登录认证反馈信息
        MessageCode msg;
        if (ack->ret == CLIENT_LoginAck::Allow) {
            msg.code = COMM_Code::LoginSuccess;
        }
        else if (ack->ret == CLIENT_LoginAck::Deny) {
            msg.code = COMM_Code::LoginFail;
        }
        else {
            return FALSE;
        }

        MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);

    }
        break;

    //心跳处理
    case COMM_FRAMETYPE_Heartbeat:
    {
        glo_comm.heartRecvWaitCount = 0;
    }
        break;

    //文件处理
    case COMM_FRAMETYPE_FileSendRequest:
    case COMM_FRAMETYPE_FileSendContent:
    case COMM_FRAMETYPE_FileTransState:
    {
        ret = FileRecvProc(type, frameLen, frame);
    }
        break;
    default:
        break;
    }

    return TRUE;
}

int CommManager::FileRecvProc(int type, int frameLen, unsigned char *frame)
{
    if (frame == NULL) {
        return FALSE;
    }

    int ret = FALSE;

    int valueLen = 0;
    unsigned char *value = NULL;

    ret = GetValueByFrame(frame, frameLen, &value, valueLen);

    CMD_FILERECV *fileRecv = &glo_comm.cmdData.fileRecv;

    switch (type) {
    case COMM_FRAMETYPE_FileSendRequest:
    {
        //判断是给服务器自己还是进行转发
        FRM_FileSendReq *v = (FRM_FileSendReq *)value;
        //check流程
        if (CheckFileRecvProcState(fileRecv, CommandState::Idle) == FALSE) {
            //1.向发送方发送接收错误帧,终止对方请求
            SendFileRecvState(CommandState::FileRecvError);
            break;
        }

        //初始化接收数据
        glo_comm.cmdData.curCmd = CommandState::FileRecv;
        fileRecv->srcId = v->destId;
        fileRecv->state = CommandState::FileRecvRecving;
        //创建文件路径
        char binPath[COMMON_STRLEN_512];
        UtilFunc::GetApplicationDirPath(binPath);
        sprintf(fileRecv->fullFilePath, "%s/data/", binPath);

        bool createFlag = UtilFunc::CreateDirPath(fileRecv->fullFilePath);
        if (createFlag == false) {
            StopFileRecvProc();
            break;
        }

        fileRecv->fileLen = v->fileLen;
        strcat_s(fileRecv->fullFilePath, COMMON_STRLEN_512, v->fileName);

        //申请内存
        int64_t btime = UtilFunc::GetSysTimeMS();

        fileRecv->fileBuf = (unsigned char *)malloc(fileRecv->fileLen);
        if (fileRecv->fileBuf == NULL) {
            assert(!"ProcFileRecvBegin malloc error!");
        }

        int64_t etime = UtilFunc::GetSysTimeMS();
        int64_t durTime = etime-btime;

        qDebug()<<"recv file request! srcId:"<<fileRecv->srcId
                <<"malloc time:"<<durTime
                <<QTime::currentTime();

        //发送给通讯模块外部
        MessageCode msg;
        msg.code = COMM_Code::CMD_FileSendRequest;
        MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
    }
        break;

    case COMM_FRAMETYPE_FileSendContent:
    {
        //判断是给服务器自己还是进行转发
        FRM_FileSendContent *v = (FRM_FileSendContent *)value;
        //check流程
        if (CheckFileRecvProcState(fileRecv, CommandState::FileRecvRecving) == FALSE) {
            StopFileRecvProc();
            break;
        }

        //赋值,判断文件末尾不能使用memcpy
        //原处理方式
        /*int lastFileLen = fileRecv->fileLen-fileRecv->recvIndex;
        if (lastFileLen < CMD_FILESEND_SINGLE_LENTH) {
            memcpy(&fileRecv->fileBuf[fileRecv->recvIndex], v->content, lastFileLen);
            fileRecv->recvIndex += lastFileLen;
        }
        else {
            memcpy(&fileRecv->fileBuf[fileRecv->recvIndex], v->content, CMD_FILESEND_SINGLE_LENTH);
            fileRecv->recvIndex += CMD_FILESEND_SINGLE_LENTH;
        }*/

        //现处理方式
        memcpy(&fileRecv->fileBuf[fileRecv->recvIndex], v->content, v->size);
        fileRecv->recvIndex += v->size;

        qDebug()<<"recv file content! srcId:"<<fileRecv->srcId
                <<QTime::currentTime();

        //发送给通讯模块外部
        MessageCode msg;
        msg.code = COMM_Code::CMD_FileRecvRecving;
        MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
    }
        break;

    case COMM_FRAMETYPE_FileTransState:
    {
        FRM_FileTransState *v = (FRM_FileTransState *)value;
        switch (v->state) {
        case CommandState::FileSendFinish:
        {
            //1.保存成文件
            UtilFunc::SaveFile(fileRecv->fullFilePath, (char *)fileRecv->fileBuf, fileRecv->fileLen, FILEOPER_MODE_NEW);

            //2.复归所有数据
            InitFileRecvProc(fileRecv);
            glo_comm.cmdData.curCmd = CommandState::Idle;

            //3.向发送端反馈接收成功信号
            SendFileRecvState(CommandState::FileRecvFinish);

            qDebug()<<"recv file finish. srcId:"<<fileRecv->srcId
                    <<QTime::currentTime();

            //发送给通讯模块外部
            MessageCode msg;
            msg.code = COMM_Code::CMD_FileSendFinish;
            MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
        }
            break;

        case CommandState::FileSendError:
        {
            //1.复归所有数据
            InitFileRecvProc(fileRecv);
            glo_comm.cmdData.curCmd = CommandState::Idle;

            qDebug()<<"recv file error! srcId:"<<fileRecv->srcId
                    <<QTime::currentTime();

            //发送给通讯模块外部
            MessageCode msg;
            msg.code = COMM_Code::CMD_FileSendError;
            MessageFrame(COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
        }
            break;

        default:
            break;
        }
    }
        break;
    default:
        break;
    }

    return TRUE;
}

int CommManager::CheckFileRecvProcState(CMD_FILERECV *fileRecv, int state)
{
    if (fileRecv == NULL) {
        return FALSE;
    }

    if (fileRecv->state == state) {
        return TRUE;
    }
    else {
        return FALSE;
    }
}

void CommManager::InitFileRecvProc(CMD_FILERECV *fileRecv)
{
    if (fileRecv == NULL) {
        return;
    }

    fileRecv->srcId = 0;
    fileRecv->overtime = 0;
    memset(fileRecv->fullFilePath, 0, COMMON_STRLEN_512);

    fileRecv->state = 0;
    fileRecv->recvIndex = 0;
    fileRecv->fileLen = 0;

    if (fileRecv->fileBuf) {
        free(fileRecv->fileBuf);
        fileRecv->fileBuf = NULL;
    }
}

void CommManager::StopFileRecvProc()
{
    //1.向发送方发送接收错误帧,终止传输过程
    SendFileRecvState(CommandState::FileRecvError);

    //2.初始化文件接收流程
    InitFileRecvProc(&glo_comm.cmdData.fileRecv);
}

int CommManager::SendFileRecvState(int state)
{
    FRM_FileTransState v;
    v.destId = glo_comm.cmdData.fileRecv.srcId;
    v.state = state;

    int ret = SendFrame(COMM_FRAMETYPE_FileTransState, sizeof(FRM_FileTransState), (unsigned char*)&v);

    return ret;
}

void CommManager::ParseAddressId(int addrId, int &uuid, int &exeId)
{
    uuid = addrId/10000;
    exeId = addrId%10000;
}

void CommManager::MakeAddressId(int uuid, int exeId, int &addrId)
{
    addrId = uuid*10000+exeId;
}

/**
 * @brief srv_rwproc    读线程
 * @param pParam
 * @return
 */
UINT srv_readproc(LPVOID pParam)
{
    CommManager *commproc = (CommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    while (glo_liveflag) {

        commproc->ReadProc();

        Sleep(COMM_READPROC_INTER);
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
    CommManager *commproc = (CommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    while (glo_liveflag) {

        commproc->ForwardingProc();

        Sleep(COMM_FORWARDING_INTER);
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
    CommManager *commproc = (CommManager *)pParam;

    if (commproc == NULL)  return 0;

    glo_threadnum ++;

    while (glo_liveflag) {

        commproc->DispatchProc();

        Sleep(COMM_DISPATCHPROC_INTER);
    }

    glo_threadnum --;
    return 0;
}
