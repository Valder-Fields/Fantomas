
#include <QDebug>
#include <QTime>

#include "../../../../utils/utils.h"
#include "../../../commsocket/commsocket.h"

#include "service.h"

FILE *saveFp = NULL;
FILE *readFp = NULL;

BaseNetService::BaseNetService()
{

}

BaseNetService::~BaseNetService()
{

}

int BaseNetService::Start(int protoType)
{
    return SCommunicator::Start(protoType);
}

int BaseNetService::Stop()
{
    return SCommunicator::Stop();
}

void BaseNetService::DispatchProc()
{
    //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int64_t btime = UtilFunc::GetSysTimeMS();

    //转发业务
    for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
        if (GetClientData(cliId)->enableFlag == FALSE) {
            continue;
        }

        for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
            //过滤未连接通道(后期加入断线重连业务时可不过滤未连接通道,防止通道突然断开后还有数据未处理)
            if (GetClientData(cliId)->commData[commId].connFlag == FALSE) {
                continue;
            }

            //1.通道接收处理
            RxChannelProc(cliId, commId);

            //2.通道发送处理(事务:回放,文件收发,心跳等)
            TxChannelProc(cliId, commId);
        }
    }

    GetServerData()->sysData.dispatchProcLoopCount++;

    //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //qint64 durTime = etime-btime;
    int64_t etime = UtilFunc::GetSysTimeMS();
    int64_t durTime = etime-btime;
    if (durTime>1) {
        if (0) qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!DispatchProc time:"<<durTime;
    }
}

int BaseNetService::RxChannelProc(int cliId, int commId)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) return FALSE;

    //过滤未连接通道,防止通道突然断开后还有数据未处理
    if (isConnected(cliId, commId) == FALSE) {
        return FALSE;
    }

    int getFrameFlag = TRUE, ret = FALSE, type = 0, valueLen = 0;
    unsigned char *value = NULL;

    while(getFrameFlag == TRUE) {
        getFrameFlag = GetAWholeFrame(cliId, commId, type, GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen);

        //处理帧
        if (getFrameFlag == TRUE) {
            switch (type) {
            //////////////////////////////////////////////////////////////////////
            //业务
            //////////////////////////////////////////////////////////////////////
            case COMM_FRAMETYPE_LoginReq: //客户端登录请求
            {
                //解析
                ret = GetValueByFrame(GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen, &value, valueLen);

                FRM_LoginReq v;
                memcpy(&v, value, valueLen);

                //客户端登录处理
                ret = DealLoginReq(cliId, commId, v);

                if (1) qDebug()<<"login!!!!! IP:"<<GetClientData(cliId)->commData[commId].ipAddr
                               <<"cliId:"<<cliId
                               <<"commId:"<<commId
                               <<"uuid:"<<v.uuid
                               <<"exeId:"<<v.exeId
                               <<"ret = "<<ret
                               <<QTime::currentTime();

                //发送给通讯模块外部
                CommonMessageCode msg;
                if (ret == TRUE) {
                    msg.code = COMM_Code::LoginSuccess;
                    CommonTimeInfo t;
                    UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
                    sprintf(msg.content, "客户端登录成功! IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                            GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                            cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                            t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
                }
                else {
                    msg.code = COMM_Code::LoginFail;
                    CommonTimeInfo t;
                    UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
                    sprintf(msg.content, "客户端登录失败! IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                            GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                            cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                            t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
                }

                MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);
            }
                break;

            //////////////////////////////////////////////////////////////////////
            //训练控制
            //////////////////////////////////////////////////////////////////////
            case COMM_FRAMETYPE_SendSet:    //下发想定
            {
                //解析
                ret = GetValueByFrame(GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen, &value, valueLen);
                if (ret == FALSE) {
                    break;
                }

                //1,处理+转发
                DealSendSet(cliId, commId, (FRM_SendSet *)value);
            }
                break;
            case COMM_FRAMETYPE_SendSetRet:     //下发想定回执
            {
                qDebug()<<QString::fromLocal8Bit("下发想定回执")<<QTime::currentTime();
                //转发
                PutSrvForwardingBuf(cliId, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
            }
                break;

            case COMM_FRAMETYPE_ControlTrain:   //控制命令
            {
                //解析
                ret = GetValueByFrame(GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen, &value, valueLen);
                if (ret == FALSE) {
                    break;
                }

                //1,处理+转发
                DealControlTrain(cliId, commId, (FRM_ControlTrain *)value);
            }
                break;

            case COMM_FRAMETYPE_ControlTrainRet://控制命令回执
            {
                //解析
                ret = GetValueByFrame(GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen, &value, valueLen);
                if (ret == FALSE) {
                    break;
                }

                //1,处理
                ret = DealControlTrainRet(cliId, commId, (FRM_ControlTrainRet *)value);

                //2,转发
                //test后期改成发向导控端
                PutSrvForwardingBuf(cliId, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
            }
                break;

            case COMM_FRAMETYPE_PlaybackRate:   //回放速率
            {
                //解析
                ret = GetValueByFrame(GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen, &value, valueLen);
                if (ret == FALSE) {
                    break;
                }

                //1,处理
                DealPlaybackRate(cliId, commId, (GL_FastForward *)value);

                //2,转发
                PutTxBufByExeId(ExeId::Effector, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
            }
                break;

            case COMM_FRAMETYPE_CallNetSrvFileDir: //召唤文件目录
            {
                //解析
                ret = GetValueByFrame(GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen, &value, valueLen);

                DealCallFileDir(cliId, commId, (FRM_CallFileDir *)value);
            }
                break;

            case COMM_FRAMETYPE_DownFile:	//下载文件
            {
                //解析
                ret = GetValueByFrame(GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen, &value, valueLen);

                DealDownFile(cliId, commId, (FRM_DownFile *)value);
            }
                break;

            case COMM_FRAMETYPE_FileSendRequest:
            case COMM_FRAMETYPE_FileSendContent:
            case COMM_FRAMETYPE_FileTransState:
            {
                //文件接收处理
                FileRecvProc(cliId, commId, type, GetRxBufPtr(cliId, commId)->onceFrameLen, GetRxBufPtr(cliId, commId)->onceFrame);
            }
                break;

            //////////////////////////////////////////////////////////////////////
            //心跳
            //////////////////////////////////////////////////////////////////////
            case COMM_FRAMETYPE_Heartbeat:
            {
                ClearHeartRecvWaitCount(cliId, commId);

                PutTxBuf(cliId, commId, GetRxBufPtr(cliId, commId)->onceFrame, GetRxBufPtr(cliId, commId)->onceFrameLen);

                if (0) qDebug()<<"recv Heartbeat!!!!!!!!!!!!!!!!!!"<<"cliId:"<<cliId<<"commId:"<<commId<<QTime::currentTime();
            }
                break;

            default:
                //test 转发所有帧
                if (0) qDebug()<<"dealframe IP:"<<GetClientData(cliId)->commData[commId].ipAddr<<"commId:"<<commId
                               <<"type:"<<type \
                               <<"content size:"<<valueLen \
                               <<QTime::currentTime();

                if (GetServerData()->forwardingMode == ForwardingMode::ServerControl) {
                    PutSrvForwardingBuf(cliId, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
                }
                else if (GetServerData()->forwardingMode == ForwardingMode::UserControl) {
                    ret = DealOriginFrame(cliId, commId, type, GetRxBufPtr(cliId, commId)->onceFrameLen, GetRxBufPtr(cliId, commId)->onceFrame);
                }

                //错误帧直接抛弃
                if (0) qDebug()<<"Parse Frame error!!!!!!!!!!!!!!!!!!";
            }

            GetServerData()->runData.recvAllFrame++;

            //注意! head+
            //移动CLIRXBUF.head指针
            GetRxBufPtr(cliId, commId)->head = (GetRxBufPtr(cliId, commId)->head + GetRxBufPtr(cliId, commId)->onceFrameLen) % COMM_RXBUF_LEN;
        }
    }

    return TRUE;
}

int BaseNetService::TxChannelProc(int cliId, int commId)
{
    //服务器自身业务处理
    int ret = 0;

    //命令处理(需要根据客户端的读取速度确定文件发送的速度,否则客户端读不过来造成bug)
    CommandProc(cliId, commId);

    //心跳
    int64_t curTime = UtilFunc::GetSysTimeMS();
    if ((curTime-GetClientData(cliId)->commData[commId].lastHeartCheckTime) > GetServerData()->heartCheckInter) {
        ret = DealHeartbeat(cliId, commId);
        GetClientData(cliId)->commData[commId].lastHeartCheckTime = curTime;
        GetServerData()->sysData.heartCheckCount++;
        if (0) qDebug()<<"DealHeartbeat!!!!!!!!!!!!!!!!!!"<<QTime::currentTime();
    }

    return ret;
}

int BaseNetService::DealLoginReq(int cliId, int commId, FRM_LoginReq &v)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) {
        return FALSE;
    }

    //1.登录验证
    //1.1 校验-系统内uuid,exeId是否冲突.如冲突,拒绝请求(防止配置错误)
    int frameLen = 0;
    unsigned char frame[COMMON_STRLEN_128];
    int ret = CheckLoginValid(cliId, commId, v.uuid, v.exeId);
    if (ret == FALSE) {
        FRM_LoginAck ack;
        ack.ret = CLIENT_LoginAck::Deny;

        SCommunicator::MakeAWholeFrame(COMM_FRAMETYPE_LoginAck, (unsigned char *)&ack, sizeof(FRM_LoginAck), frame, frameLen);

        PutTxBuf(cliId, commId, frame, frameLen);
        return FALSE;
    }

    //1.2 向该客户端发送通过请求帧
    FRM_LoginAck ack;
    ack.ret = CLIENT_LoginAck::Allow;

    SCommunicator::MakeAWholeFrame(COMM_FRAMETYPE_LoginAck, (unsigned char *)&ack, sizeof(FRM_LoginAck), frame, frameLen);

    PutTxBuf(cliId, commId, frame, frameLen);

    //2.赋值客户端exeId
    GetClientData(cliId)->commData[commId].exeId = v.exeId;

    //3.如果是客户端或控制端,则向控制端更新客户端状态
    //判断:如果是控制端,返回所有客户端状态
    if (v.exeId == ExeId::SysController) {
        for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
            if (GetClientData(cliId)->enableFlag == FALSE) {
                continue;
            }

            for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
                //过滤未连接通道
                if (GetClientData(cliId)->commData[commId].connFlag == FALSE ||
                    GetClientData(cliId)->commData[commId].socket <= 0) {
                    continue;
                }

                //如当前通道为客户端,则向控制端发送客户端状态(此处肯定是客户端上线)
                if (GetClientData(cliId)->commData[commId].exeId == ExeId::SysClient) {
                    FRM_ClientState state;
                    state.uuid = GetClientData(cliId)->uuid;
                    state.state = CLIENT_State::Online;

                    SCommunicator::MakeAWholeFrame(COMM_FRAMETYPE_ClientState, (unsigned char *)&state, sizeof(FRM_ClientState), frame, frameLen);

                    PutTxBufByExeId(ExeId::SysController, frame, frameLen);

                    break; //跳出通道循环
                }
            }
        }
    }
    //如果是客户端,向控制端返回本客户端状态
    else if (v.exeId == ExeId::SysClient) {
        FRM_ClientState state;
        state.uuid = GetClientData(cliId)->uuid;
        state.state = CLIENT_State::Online;

        SCommunicator::MakeAWholeFrame(COMM_FRAMETYPE_ClientState, (unsigned char *)&state, sizeof(FRM_ClientState), frame, frameLen);

        PutTxBufByExeId(ExeId::SysController, frame, frameLen);
    }
    else {
        //其他程序请求则不更新状态
    }

    return TRUE;
}

int BaseNetService::CheckLoginValid(int curCliId, int curCommId, int uuid, int exeId)
{
    if (curCliId < 0 || curCliId >= COMM_CLI_NUM || curCommId < 0 || curCommId >= COMM_CLICOMM_NUM) {
        return FALSE;
    }

    //检查不同客户端uuid是否重复
    for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
        if (GetClientData(cliId)->enableFlag == FALSE) {
            continue;
        }

        if (cliId == curCliId) {
            continue;
        }

        if (GetClientData(cliId)->uuid == uuid) {
            return FALSE;
        }
    }

    //检查该客户端exeId是否有多个通道(客户端断线后,服务器未复归该客户端数据之前,客户端又连上了),如重复则复归其通道数据
    for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
        if (GetClientData(curCliId)->commData[commId].connFlag == FALSE) {
            continue;
        }

        //跳过当前通道
        if (commId == curCommId) {
            continue;
        }

        if (GetClientData(curCliId)->commData[commId].exeId == exeId) {
            //执行旧通道断线流程
            BaseNetService::CommBrokenProc(curCliId, commId);
        }
    }

    return TRUE;
}

int BaseNetService::DealHeartbeat(int cliId, int commId)
{
    //1.心跳发送(服务器无需发送)
    /*if (isHeartSendTimeout(cliId, commId) == TRUE) {
        unsigned char buffer[COMMON_STRLEN_32];
        int len = 0;

        Heartbeat v;
        v.state = 1;

        m_protocol->MakeAWholeFrame(COMM_FRAMETYPE_Heartbeat, sizeof(v), (unsigned char *)&v, buffer, len);
        PutTxBuf(cliId, commId, buffer, len);

        GetClientData(cliId)->commData[commId].heartSendWaitCount = 0;
    }

    GetClientData(cliId)->commData[commId].heartSendWaitCount++;*/

    //2.心跳接收
    if (BaseNetService::isHeartRecvTimeout(cliId, commId) == TRUE) {    //判断超时
        if (BaseNetService::isConnected(cliId, commId) == TRUE) {  //判断是否已断线
            //1.执行断线流程
            BaseNetService::CommBrokenProc(cliId, commId);
        }
    }

    GetClientData(cliId)->commData[commId].heartRecvWaitCount++;
    if (0) qDebug()<<"heartRecvWaitCount+++++++++++"<<QTime::currentTime();

    return TRUE;
}

void BaseNetService::ServiceProc()
{
    //保存,回放业务
    for (int i = 0; i < COMM_SRVROOM_NUM; i++) {
        if (GetServerData()->roomData[i].useFlag == FALSE) {
            continue;
        }

        SaveProcByRoomId(i);
        PlayProcByRoomId(i);
    }

}

NETSRV_SRVSAVE * BaseNetService::GetSrvSaveDataPtrByRoomId(int roomId)
{
    if (roomId < 0 || roomId >= COMM_SRVROOM_NUM) {
        return NULL;
    }

    return &GetServerData()->roomData[roomId].saveData;
}

int BaseNetService::SaveProcByRoomId(int roomId)
{
    if (roomId < 0 || roomId >= COMM_SRVROOM_NUM) {
        return NETSRV_ERR_SAVE_Common;
    }

    if (GetServerData()->roomData[roomId].saveData.useFlag == FALSE) {
        return NETSRV_ERR_SAVE_Common;
    }

    if (0) qDebug()<<"SaveProcByRoomId:"<<roomId;

    NETSRV_SRVSAVE  *saveData = GetSrvSaveDataPtrByRoomId(roomId);
    if (saveData == NULL) {
        //依据判例一,此处应注释掉assert
        //assert(!"GetSrvSaveDataPtrByRoomId error!");
        return NETSRV_ERR_SAVE_Common;
    }

    SRV_COMM_CHANNEL_TXBUF *txbuf = GetSrvTxBufPtrByRoomId(roomId);
    if (txbuf == NULL) {
        //依据判例一,此处应注释掉assert
        //assert(!"GetSrvTxBufPtrByRoomId error!");
        return NETSRV_ERR_SAVE_Common;
    }

    if (saveData->lockFlag == TRUE) {
        saveData->lockFlag = FALSE; //解锁其他线程
    }

    //赋值
    int curInfoIndex = saveData->procData.curInfoIndex; //当前时刻队列索引(0~999)
    saveData->procData.rcdInfoQueue[curInfoIndex].rcdIndex = saveData->procData.procCount; //当前记录条数(时间)(0~xxxxxx)
    saveData->procData.rcdInfoQueue[curInfoIndex].bufIndex = txbuf->tail; //当前服务器转发缓存区指针(tail)

    int lastInfoIndex = (curInfoIndex+NETSRV_SRVSAVE_RCDINFONUM-1)%NETSRV_SRVSAVE_RCDINFONUM;
    int rcdLength = (saveData->procData.rcdInfoQueue[curInfoIndex].bufIndex+COMM_SRVTXBUF_LEN-saveData->procData.rcdInfoQueue[lastInfoIndex].bufIndex)%COMM_SRVTXBUF_LEN;

    saveData->procData.rcdInfoQueue[curInfoIndex].rcdLength = rcdLength;

    int onceInfoCount = GetServerData()->saveFileInter/GetServerData()->serviceProcInter;

    //1.正常保存,每隔60(1800/30)条记录的时间周期保存一次
    if ((saveData->procData.procCount%onceInfoCount) == 0) {
        //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        int64_t btime = UtilFunc::GetSysTimeMS();

        int beginInfoIndex = (curInfoIndex+NETSRV_SRVSAVE_RCDINFONUM-onceInfoCount)%NETSRV_SRVSAVE_RCDINFONUM;

        //60条记录时间周期
        int ret = SaveTrainFileByProcData(saveData->fileName, beginInfoIndex, onceInfoCount, &saveData->procData, txbuf);
        if (ret == FALSE) {
            assert(!"SaveTrainFileByProcData error!");
        }
        //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
        //qint64 durTime = etime-btime;
        int64_t etime = UtilFunc::GetSysTimeMS();
        int64_t durTime = etime-btime;
        if ((durTime)>=0) {
            if (0) qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!SaveProcByRoomId durTime:"<<durTime<<QTime::currentTime();
        }
    }

    //索引递增
    saveData->procData.curInfoIndex = (curInfoIndex+1)%NETSRV_SRVSAVE_RCDINFONUM;
    saveData->procData.procCount++;

    return TRUE;
}

int BaseNetService::SaveTrainFileByProcData(char *fileName, int beginInfoIndex, int infoNums, NETSRV_SAVEPROCDATA *procData, SRV_COMM_CHANNEL_TXBUF *txbuf)
{
    if (fileName == NULL || beginInfoIndex < 0 || infoNums < 0 || procData == NULL || txbuf == NULL) {
        return FALSE;
    }

    //未产生新数据,直接返回
    if (procData->rcdInfoQueue[procData->curInfoIndex].bufIndex == procData->rcdInfoQueue[beginInfoIndex].bufIndex) {
        return TRUE;
    }

    errno_t err;
    err = fopen_s(&saveFp, fileName, "ab+");
    if (err != 0) {
        return FALSE;
    }

    int ret = 0;
    int curInfoIndex = beginInfoIndex;
    unsigned char *saveBuf = procData->saveBuf;

    for (int i = 0; i < infoNums; i++) {
        //写入一条记录(ID+LEN+VAL)
        int rcdLength = procData->rcdInfoQueue[curInfoIndex].rcdLength;
        //记录有值才写入
        if (rcdLength > 0) {
            int a = procData->rcdInfoQueue[curInfoIndex].rcdIndex;
            //ID
            //saveBuf[0] = (a & 0xff);
            //saveBuf[1] = ((a & 0xff00) >> 8);
            //saveBuf[2] = ((a & 0xff0000) >> 16);
            //saveBuf[3] = ((a & 0xff000000) >> 24);
            saveBuf[3] = a >> 24;
            saveBuf[2] = a >> 16;
            saveBuf[1] = a >> 8;
            saveBuf[0] = a;

            //LEN
            a = rcdLength;
            saveBuf[7] = a >> 24;
            saveBuf[6] = a >> 16;
            saveBuf[5] = a >> 8;
            saveBuf[4] = a;

            if (rcdLength > NETSRV_SRVSAVE_BUFLENTH) {
                assert(!"SaveTrainFileByProcData: rcdLength > NETSRV_SRVSAVE_BUFLENTH !");
            }

            //VAL
            int saveBufIndex = NETSRV_SRVSAVE_BYTES_ID+NETSRV_SRVSAVE_BYTES_LENTH;

            int lastInfoIndex = (curInfoIndex+NETSRV_SRVSAVE_RCDINFONUM-1)%NETSRV_SRVSAVE_RCDINFONUM;

            int beginTxBufIndex = procData->rcdInfoQueue[lastInfoIndex].bufIndex;

            if (procData->rcdInfoQueue[curInfoIndex].bufIndex > procData->rcdInfoQueue[lastInfoIndex].bufIndex) {
                memcpy((saveBuf+saveBufIndex), &txbuf->buf[beginTxBufIndex], rcdLength);
            }
            else {
                for (int j = 0; j < rcdLength; j++) {
                    saveBuf[saveBufIndex+j] = txbuf->buf[beginTxBufIndex];
                    beginTxBufIndex = (beginTxBufIndex+1)%COMM_SRVTXBUF_LEN;
                }
            }

            procData->saveBufLen = saveBufIndex + procData->rcdInfoQueue[curInfoIndex].rcdLength;
            ret = (int)fwrite(saveBuf, procData->saveBufLen, 1, saveFp);
            if (ret != 1) { //此处的1为fwrite第三个参数
                qDebug()<<"fwrite error!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
            }
        }
        else {

        }

        curInfoIndex = (curInfoIndex+1)%NETSRV_SRVSAVE_RCDINFONUM;
    }

    fclose(saveFp);

    return TRUE;
}

int BaseNetService::SaveTrainFile(const char *fileName, unsigned char *buffer, int bufferLen)
{
    if (fileName == NULL || buffer == NULL || bufferLen <= 0) {
        return FALSE;
    }

    errno_t err;
    err = fopen_s(&saveFp, fileName, "ab+");
    if (err != 0) {
        return FALSE;
    }

    fwrite(buffer, bufferLen, 1, saveFp);
    fclose(saveFp);
    saveFp = NULL;

    return TRUE;
}

NETSRV_SRVPLAY * BaseNetService::GetSrvPlayDataPtrByRoomId(int roomId)
{
    if (roomId < 0 || roomId >= COMM_SRVROOM_NUM) {
        return NULL;
    }

    return &GetServerData()->roomData[roomId].playData;
}

int BaseNetService::PlayProcByRoomId(int roomId)
{
    if (roomId < 0 || roomId >= COMM_SRVROOM_NUM) {
        return NETSRV_ERR_PLAY_Common;
    }

    if (GetServerData()->roomData[roomId].playData.useFlag == FALSE) {
        return NETSRV_ERR_PLAY_Common;
    }

    if (0) qDebug()<<"PlayProcByRoomId:"<<roomId;

    NETSRV_SRVPLAY  *playData = GetSrvPlayDataPtrByRoomId(roomId);
    if (playData == NULL) {
        //依据判例一,此处应注释掉assert
        //assert(!"GetSrvPlayDataPtrByRoomId error!");
        return NETSRV_ERR_PLAY_Common;
    }

    //qint64 btime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    int64_t btime = UtilFunc::GetSysTimeMS();

    switch(playData->state) {
    case ControlTrain::Init:
    {
        //第一次要获取文件字节总数
        int ret = 0;
        ret = UtilFunc::GetFileLength(readFp, playData->fileName, playData->fileLen);
        if (ret == FALSE) {
            playData->state = ControlTrain::Error;
            return NETSRV_ERR_PLAY_Common;
        }

        if (playData->fileLen >= NETSRV_SRVPLAY_FILELEN_LMT) {  //方案2:读取内容到缓存
            playData->mode = NETSRV_SRVPLAY_MODE_BUFF;
            playData->bufLen = NETSRV_SRVPLAY_BUFLEN;
        }
        else {
            playData->mode = NETSRV_SRVPLAY_MODE_READALL;   //方案1:一次性读取全部内容
            playData->bufLen = playData->fileLen;
        }

        //读取内容
        switch (playData->mode) {
        case NETSRV_SRVPLAY_MODE_BUFF:
        {

        }
            break;
        case NETSRV_SRVPLAY_MODE_READALL:
        {
            int64_t btime1 = UtilFunc::GetSysTimeMS();

            playData->procData.fileBuf = (unsigned char *)malloc(playData->fileLen);
            if (playData->procData.fileBuf == NULL) {
                qDebug()<<"PlayProcByRoomId malloc error! roomId:"<<roomId<<QTime::currentTime();
                assert(!"PlayProcByRoomId malloc error!");
            }

            ret = UtilFunc::ReadFile(readFp, playData->fileName, playData->procData.fileBuf, playData->fileLen);
            if (ret == FALSE) {
                playData->state = ControlTrain::Error;
                return NETSRV_ERR_PLAY_Common;
            }

            int64_t etime1 = UtilFunc::GetSysTimeMS();
            int64_t durTime1 = etime1-btime1;

            if (durTime1 >= 0) {
                qDebug()<<"malloc and read file time! roomId:"<<roomId<<"durTime:"<<durTime1<<QTime::currentTime();
            }
        }
            break;
        default:

            break;
        }

        playData->state = ControlTrain::Start;
    }
        break;

    case ControlTrain::Start:
    case ControlTrain::Continue:
    {
        //如使用方案2,添加分步读取步骤,并设定每次读取的条数

        //解析(解析条数=发送条数)
        //int64_t a1 = UtilFunc::GetSysTimeMS();

        int ret = 0;
        ret = ParseOnceTrainRecord(roomId);
        if (ret == NETSRV_SRVPLAY_PARSE_ERROR) {
            playData->state = ControlTrain::Error;
            return NETSRV_ERR_PLAY_Common;
        }

        //int64_t a2 = UtilFunc::GetSysTimeMS();
        //int64_t a21 = a2-a1;
        //Valder-Fields qDebug()<<"...................play ParseOnceTrainRecord:"<<a21<<QTime::currentTime();

        //通过回放倍速控制,将数据放入客户端的发送队列
        ret = PlayBackOnceTrainRecord(roomId);

        //回放转发完毕,过程结束
        if (ret == NETSRV_SRVPLAY_SEND_FINISH) {
             playData->state = ControlTrain::Stop;
        }

        //int64_t a3 = UtilFunc::GetSysTimeMS();
        //int64_t a32 = a3-a2;
        //Valder-Fields qDebug()<<"...................play PlayBackOnceTrainRecord:"<<a32<<QTime::currentTime();

        //Valder-Fields qDebug()<<"...................play indexCount:"<<playData->procData.indexCount;
    }
        break;

    case ControlTrain::Pause:
    {
        //do nothing
    }
        break;

    case ControlTrain::Error:
    {
        //出现错误,应返回发送一个错误帧告知控制端,同时关闭回放流程
        playData->state = ControlTrain::Stop;

        //如已申请内存,则应释放内存
        if (playData->fileLen > 0) {
            if (playData->procData.fileBuf) {
                free(playData->procData.fileBuf);
                playData->procData.fileBuf = NULL;
            }
        }
    }
        break;

    case ControlTrain::Stop:
    {
        qDebug()<<"...................play finish! indexCount:"<<playData->procData.indexCount<<QTime::currentTime();;

        //1.初始化PlayData
        if (playData->procData.fileBuf) {
            free(playData->procData.fileBuf);
            playData->procData.fileBuf = NULL;
        }

        memset(playData, 0, sizeof(NETSRV_SRVPLAY));

        //2.告知控制端,回放已结束,控制端收到结束状态后,发送结束训练命令,这样的方式可以兼容普通训练
        FRM_ControlTrainRet v;
        memset(&v, 0, sizeof(FRM_ControlTrainRet));
        v.uuid = 0;
        v.exeId = ExeId::NetServer;
        v.trainType = TrainType::Playback;
        v.trainState = ControlTrain::Stop;

        int  bufferLen = 0;
        unsigned char buffer[COMMON_STRLEN_512];

        //3,发送给导控端
        SCommunicator::MakeAWholeFrame(COMM_FRAMETYPE_SControlTrainRet, (unsigned char *)&v, sizeof(FRM_ControlTrainRet), buffer, bufferLen);

        //!!! 后边等赵星改完后变成单播
        PutTxBufByExeId(ExeId::TrainController, buffer, bufferLen);

        PutSrvForwardingBufByRoomId(0, buffer, bufferLen);
    }
        break;

    default:
        break;
    }

    //qint64 etime = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //qint64 durTime = etime-btime;
    int64_t etime = UtilFunc::GetSysTimeMS();
    int64_t durTime = etime-btime;
    if ((durTime)>=0) {
        if (0) qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!!!!PlayProcByRoomId roomId:"<<roomId<<"durTime:"<<durTime<<QTime::currentTime();
    }

    return TRUE;
}

int BaseNetService::ParseOnceTrainRecord(int roomId)
{
    NETSRV_SRVPLAY *playData = GetSrvPlayDataPtrByRoomId(roomId);
    if (playData == NULL) {
        //依据判例一,此处应注释掉assert
        //assert(!"GetSrvPlayDataPtrByRoomId error!");
        return NETSRV_SRVPLAY_PARSE_ERROR;
    }

    int curDiffNums = 0; //回放队列状态判断
    int ret = 0;

    for (int i = 0; i < playData->rate; i++) {
        curDiffNums = (playData->procData.tailInfoIndex+NETSRV_SRVPLAY_INFONUM-playData->procData.headInfoIndex)%NETSRV_SRVPLAY_INFONUM;
        if (curDiffNums == (NETSRV_SRVPLAY_INFONUM-1)) { //tail追上head,不继续解析
            return NETSRV_SRVPLAY_PARSE_WAIT;
        }

        if (curDiffNums < (NETSRV_SRVPLAY_INFONUM-1)) { //tail没追上head,进行解析
            if (curDiffNums < 0) {
                if (1) qDebug()<<"ParseOnceTrainRecord error! part2 curDiffNums:"<<curDiffNums;
                return NETSRV_SRVPLAY_PARSE_ERROR;
            }
            ret = ParseOneTrainRecord(roomId);
        }
        else if (curDiffNums > NETSRV_SRVPLAY_INFONUM) {
            if (1) qDebug()<<"ParseOnceTrainRecord error! part3 curDiffNums:"<<curDiffNums;
            return NETSRV_SRVPLAY_PARSE_ERROR;
        }
    }

    return ret;
}

int BaseNetService::ParseOneTrainRecord(int roomId)
{
    NETSRV_SRVPLAY *playData = GetSrvPlayDataPtrByRoomId(roomId);
    if (playData == NULL) {
        //依据判例一,此处应注释掉assert
        //assert(!"GetSrvPlayDataPtrByRoomId error!");
        return NETSRV_SRVPLAY_PARSE_ERROR;
    }

    switch (playData->mode) {
    case NETSRV_SRVPLAY_MODE_BUFF:
    {

    }
        break;
    case NETSRV_SRVPLAY_MODE_READALL:
    {
        if (playData->procData.fileBufIndex >= playData->fileLen) {
            return NETSRV_SRVPLAY_PARSE_FINISH;
        }
        int fileBufIndex = playData->procData.fileBufIndex;
        int tailInfoIndex = playData->procData.tailInfoIndex;
        unsigned char *buffer = &playData->procData.fileBuf[fileBufIndex];

        int rcdIndex = buffer[0]+buffer[1]*256+buffer[2]*256*256+buffer[3]*256*256*256;
        int rcdLength = buffer[4]+buffer[5]*256+buffer[6]*256*256+buffer[7]*256*256*256;
        //debug
        if (rcdIndex > NETSRV_SRVPLAY_RCD_MAXNUM || rcdIndex < 0) {
            assert(!"ParseOneTrainRecord ERROR! rcdIndex > 2000000 || rcdIndex < 0!");

            return NETSRV_SRVPLAY_PARSE_ERROR;
        }
        if (rcdLength > playData->fileLen || rcdLength < 0) {
            assert(!"ParseOneTrainRecord ERROR! rcdLength > fileLength || rcdLength < 0!");

            return NETSRV_SRVPLAY_PARSE_ERROR;
        }
        playData->procData.playInfoQueue[tailInfoIndex].rcdIndex  = rcdIndex;
        playData->procData.playInfoQueue[tailInfoIndex].rcdLength = rcdLength;
        playData->procData.playInfoQueue[tailInfoIndex].bufIndex  = fileBufIndex+NETSRV_SRVSAVE_BYTES_ID+NETSRV_SRVSAVE_BYTES_LENTH;

        //Valder-Fields qDebug()<<"ParseOneTrainRecord tailInfoIndex:"<<tailInfoIndex;

        playData->procData.tailInfoIndex = (playData->procData.tailInfoIndex+1)%NETSRV_SRVPLAY_INFONUM;
        //todo 注意,如为模式1,则需要模缓存长度
        playData->procData.fileBufIndex  = (playData->procData.fileBufIndex+NETSRV_SRVSAVE_BYTES_ID+NETSRV_SRVSAVE_BYTES_LENTH+playData->procData.playInfoQueue[tailInfoIndex].rcdLength);

        if (playData->procData.fileBufIndex >= playData->fileLen) {
            return NETSRV_SRVPLAY_PARSE_FINISH;
        }
        //Valder-Fields qDebug()<<"ParseOneTrainRecord rcdIndex:"<<rcdIndex;
        //Valder-Fields qDebug()<<"ParseOneTrainRecord rcdLength:"<<rcdLength;
        //Valder-Fields qDebug()<<"ParseOneTrainRecord nextFileBufIndex:"<<playData->procData.fileBufIndex;
    }
        break;
    default:

        break;
    }

    return NETSRV_SRVPLAY_PARSE_RUNNING;
}

int BaseNetService::PlayBackOnceTrainRecord(int roomId)
{
    NETSRV_SRVPLAY *playData = GetSrvPlayDataPtrByRoomId(roomId);
    if (playData == NULL) {
        //依据判例一,此处应注释掉assert
        //assert(!"GetSrvPlayDataPtrByRoomId error!");
        return NETSRV_SRVPLAY_SEND_ERROR;
    }

    //从队列头部取出记录进行回放转发
    for (int i = 0; i < playData->rate; i++) {
        if (playData->procData.headInfoIndex == playData->procData.tailInfoIndex) {
            //播放流程结束
            return NETSRV_SRVPLAY_SEND_FINISH;
        }

        int headIndex = playData->procData.headInfoIndex;
        int rcdIndex  = playData->procData.playInfoQueue[headIndex].rcdIndex;

        if (playData->procData.indexCount >= rcdIndex) {    //循环时间到达记录时间,播放
            int rcdLength  = playData->procData.playInfoQueue[headIndex].rcdLength;
            int bufIndex   = playData->procData.playInfoQueue[headIndex].bufIndex;

            switch (playData->mode) {
            case NETSRV_SRVPLAY_MODE_BUFF:
            {
                //todo 如为方案2,则需要从文件缓存中逐个获取char放到CliSendBuf中
            }
                break;
            case NETSRV_SRVPLAY_MODE_READALL:
            {
                unsigned char *buffer = &playData->procData.fileBuf[bufIndex];
                PutSrvForwardingBufByRoomId(roomId, buffer, rcdLength);
            }
                break;
            default:

                break;
            }

            playData->procData.headInfoIndex = (playData->procData.headInfoIndex+1)%NETSRV_SRVPLAY_INFONUM; //head+1
        }

        playData->procData.indexCount++; //播放记录+1
    }

    return NETSRV_SRVPLAY_SEND_RUNNING;
}

int BaseNetService::DealSendSet(int cliId, int commId, FRM_SendSet *v)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM || v == NULL) {
        return FALSE;
    }

    //1,设置本地数据
    int roomId = GetClientData(cliId)->roomId;

    //test
    //获得文件绝对路径
    char filePath[512];
    char fullFilePath[512];
    UtilFunc::GetApplicationDirPath(filePath);

    sprintf(fullFilePath, "%s%s%s", filePath, NETSRV_SRVSAVE_FILEPATH, "test.prdb");

    if (v->trainType == TrainType::Playback) { //初始化回放流程数据
        sprintf(GetServerData()->roomData[roomId].playData.fileName, "%s", fullFilePath);
        //!!! 待设置文件保存路径
        //异常处理,如控制端未配置用户信息,则回放临时文件
        /*if (strcmp(v->path, "") == NULL) {

        }
        else {
            sprintf(GetServerData()->roomData[roomId].playData.fileName, "%s%s%s",filePath.toLocal8Bit().constData(), NETSRV_SRVSAVE_FILEPATH, v.fileName);
        }*/

        GetServerData()->roomData[roomId].playData.state = ControlTrain::Init;
        GetServerData()->roomData[roomId].playData.rate = GetServerData()->playRate;

        //2,转发
        PutTxBufByExeId(ExeId::Effector, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
    }
    else if (v->trainType == TrainType::Common) { //初始化存储流程数据
        //test 每次保存就删除之前的test.prdb
        UtilFunc::DelFile(fullFilePath);
        sprintf(GetServerData()->roomData[roomId].saveData.fileName, "%s", fullFilePath);

        //!!! 待设置文件保存路径
        //异常处理,如控制端未配置用户信息,则保存存储过程到临时文件
        /*if (strcmp(v.fileName, "") == NULL) {

        }
        else {
            sprintf(GetServerData()->roomData[roomId].saveData.fileName, "%s%s%s", filePath.toLocal8Bit().constData(), NETSRV_SRVSAVE_FILEPATH, v.fileName);
        }*/

        //2,转发(将当前通道onceFrame进行转发,有点不规范,后续待解决)
        PutTxBufByExeId(ExeId::Trainer, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
    }

    if (1) {
        if (v->trainType == TrainType::Common) {
            qDebug()<<QString::fromLocal8Bit("下发想定 类型:普通训练")<<QTime::currentTime();
        }
        else if (v->trainType == TrainType::Playback) {
            qDebug()<<QString::fromLocal8Bit("下发想定 类型:回放")<<QTime::currentTime();
        }
    }

    return TRUE;
}

int BaseNetService::DealControlTrain(int cliId, int commId, FRM_ControlTrain *v)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM || v == NULL) {
        return FALSE;
    }

    int roomId = GetClientData(cliId)->roomId;

    //1,处理
    switch (v->procedure) {
    case ControlTrain::Start:
    {

    }
        break;
    case ControlTrain::Pause:
    case ControlTrain::Continue:
    case ControlTrain::Stop:
    {
        if (v->trainType == TrainType::Playback) {
            GetServerData()->roomData[roomId].playData.state = v->procedure;
        }
        else if (v->trainType == TrainType::Common) {

        }
    }
        break;

    default:
        break;
    }

    //2,转发
    switch (v->trainType) {
    case TrainType::Playback:
        PutTxBufByExeId(ExeId::Effector, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
        break;
    case TrainType::Common:
        PutTxBufByExeId(ExeId::Trainer, GetClientData(cliId)->commData[commId].rxbuf.onceFrame, GetClientData(cliId)->commData[commId].rxbuf.onceFrameLen);
        break;
    default:
        break;
    }

    if (1) {
        if (v->trainType == TrainType::Common) {
            qDebug()<<QString::fromLocal8Bit("下发控制 类型:普通训练 命令:%1").arg(QString::number(v->procedure))<<QTime::currentTime();
        }
        else if (v->trainType == TrainType::Playback) {
            qDebug()<<QString::fromLocal8Bit("下发控制 类型:回放 命令:%1").arg(QString::number(v->procedure))<<QTime::currentTime();
        }
    }

    return TRUE;
}

int BaseNetService::DealControlTrainRet(int cliId, int commId, FRM_ControlTrainRet *v)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM || v == NULL) {
        return FALSE;
    }

    int roomId = GetClientData(cliId)->roomId;

    switch (v->trainState) {
    case ControlTrain::Start:
    {
        if (v->trainType == TrainType::Playback) {
            //需判断是否所有回放观察者都已就绪,目前只有一个观察者,故省略

            //启动回放,交由服务线程执行回放服务
            GetServerData()->roomData[roomId].playData.useFlag = TRUE;
        }
        else if (v->trainType == TrainType::Common) {
            GetServerData()->roomData[roomId].saveData.useFlag = TRUE;  //notice 注意:应先赋值文件名再进入保存流程,否则bug
            GetServerData()->roomData[roomId].saveData.lockFlag = TRUE;

            //notice 立即使用自旋锁阻塞式同步保存线程,待保存线程启动后再继续往下运行,是为了防止u3d响应太快,有些帧没等保存线程运行到就发出了.(在zjc项目中使用,在tz项目中下发想定和开始是两个步骤,故下发想定到开始训练时间差原则上会比较久,保存线程肯定已经启动了)
            while(GetServerData()->roomData[roomId].saveData.lockFlag) { //todo 阻塞式同步存储线程(待改进)
                Sleep(GetServerData()->serviceProcInter);
            }
        }
    }
        break;
    case ControlTrain::Stop:
    {
        if (v->trainType == TrainType::Playback) {
            GetServerData()->roomData[roomId].playData.useFlag = FALSE;
        }
        else if (v->trainType == TrainType::Common) {
            //停止保存数据
            GetServerData()->roomData[roomId].saveData.useFlag = FALSE;
        }
    }
        break;

    default:
        break;
    }

    if (1) {
        if (v->trainType == TrainType::Common) {
            qDebug()<<QString::fromLocal8Bit("控制回执 类型:普通训练 命令:%1 uuid:%2 exeId:%3").arg(v->trainState).arg(v->uuid).arg(v->exeId)<<QTime::currentTime();
        }
        else if (v->trainType == TrainType::Playback) {
            qDebug()<<QString::fromLocal8Bit("控制回执 类型:回放 命令:%1 uuid:%2 exeId:%3").arg(v->trainState).arg(v->uuid).arg(v->exeId)<<QTime::currentTime();
        }
    }

    return TRUE;
}

int BaseNetService::DealPlaybackRate(int cliId, int commId, GL_FastForward *v)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM || v == NULL) {
        return FALSE;
    }

    int roomId = GetClientData(cliId)->roomId;

    GetServerData()->roomData[roomId].playData.rate = v->rate;

    qDebug()<<QString::fromLocal8Bit("下发控制 类型:回放 命令:加速 rate:%1").arg(QString::number(v->rate))<<QTime::currentTime();

    return TRUE;
}

int BaseNetService::FileRecvProc(int cliId, int commId, int type, int frameLen, unsigned char *frame)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM || frame == NULL) {
        return FALSE;
    }

    int ret = FALSE;

    //解析
    int valueLen = 0;
    unsigned char *value = NULL;
    ret = GetValueByFrame(frame, frameLen, &value, valueLen);

    CMD_FILERECV *fileRecv = &GetClientData(cliId)->commData[commId].cmdData.fileRecv;

    switch (type) {
    case COMM_FRAMETYPE_FileSendRequest:
    {
        FRM_FileSendReq *v = (FRM_FileSendReq *)value;

        int uuid = 0, exeId = 0;
        ParseAddressId(v->destId, uuid, exeId);

        //发给服务器
        if (exeId == ExeId::NetServer) {
            //check流程
            if (CheckFileRecvProcState(fileRecv, CommandState::Idle) == FALSE) {
                //1.向发送方发送接收错误帧,终止对方请求
                SendFileRecvState(cliId, commId, CommandState::FileRecvError);
                break;
            }

            //初始化接收数据
            GetClientData(cliId)->commData[commId].cmdData.curCmd = CommandState::FileRecv;
            fileRecv->srcId = v->destId;
            fileRecv->state = CommandState::FileRecvRecving;
            //创建文件路径
            char binPath[COMMON_STRLEN_512];
            UtilFunc::GetApplicationDirPath(binPath);
            sprintf(fileRecv->fullFilePath, "%s/data/", binPath);

            bool createFlag = UtilFunc::CreateDirPath(fileRecv->fullFilePath);
            if (createFlag == false) {
                StopFileRecvProc(cliId, commId);
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

            qDebug()<<"recv file request! cliId:"<<cliId
                    <<"commId:"<<commId
                    <<"uuid:"<<GetClientData(cliId)->uuid
                    <<"malloc time:"<<durTime
                    <<QTime::currentTime();

            //发送给通讯模块外部
            CommonMessageCode msg;
            msg.code = COMM_Code::CMD_FileSendRequest;
            CommonTimeInfo t;
            UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
            sprintf(msg.content, "收到对服务器的文件发送请求. IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                    GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                    cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                    t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
            MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);
        }
        else {
            //1,转发
            SendByDestId(v->destId, frame, frameLen);
        }
    }
        break;

    case COMM_FRAMETYPE_FileSendContent:
    {
        FRM_FileSendContent *v = (FRM_FileSendContent *)value;
        int uuid = 0, exeId = 0;
        ParseAddressId(v->destId, uuid, exeId);

        //发给服务器
        if (exeId == ExeId::NetServer) {
            //check流程
            if (CheckFileRecvProcState(&GetClientData(cliId)->commData[commId].cmdData.fileRecv, CommandState::FileRecvRecving) == FALSE) {
                StopFileRecvProc(cliId, commId);
                break;
            }

            //赋值,判断文件末尾不能使用memcpy
            //原处理方式
            /*int lastFileLen = fileRecv->fileLen-fileRecv->recvIndex;
            if (lastFileLen < CMD_FILESEND_SINGLE_LENTH) {
                memcpy(&fileRecv->fileBuf[fileRecv->recvIndex], v->content, lastFileLen);
            }
            else {
                memcpy(&fileRecv->fileBuf[fileRecv->recvIndex], v->content, CMD_FILESEND_SINGLE_LENTH);
                fileRecv->recvIndex += CMD_FILESEND_SINGLE_LENTH;
            }*/

            //现处理方式
            memcpy(&fileRecv->fileBuf[fileRecv->recvIndex], v->content, v->size);
            fileRecv->recvIndex += v->size;

            qDebug()<<"recv file content! cliId:"<<cliId
                    <<"commId:"<<commId
                    <<"uuid:"<<GetClientData(cliId)->uuid
                    <<QTime::currentTime();

            //发送给通讯模块外部
            MessageCode msg;
            msg.code = COMM_Code::CMD_FileRecvRecving;
            MessageFrame(cliId, commId, COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
        }
        else {
            //1,转发
            SendByDestId(v->destId, frame, frameLen);
        }
    }
        break;

    case COMM_FRAMETYPE_FileTransState:
    {
        FRM_FileTransState *v = (FRM_FileTransState *)value;
        int uuid = 0, exeId = 0;
        ParseAddressId(v->destId, uuid, exeId);

        //发给服务器
        if (exeId == ExeId::NetServer) {
            switch (v->state) {
            case CommandState::FileSendFinish:
            {
                //1.保存成文件
                UtilFunc::SaveFile(fileRecv->fullFilePath, (char *)fileRecv->fileBuf, fileRecv->fileLen, FILEOPER_MODE_NEW);

                //2.复归所有数据
                InitFileRecvProc(&GetClientData(cliId)->commData[commId].cmdData.fileRecv);
                GetClientData(cliId)->commData[commId].cmdData.curCmd = CommandState::Idle;

                //3.向发送端反馈接收成功信号
                SendFileRecvState(cliId, commId, CommandState::FileRecvFinish);

                qDebug()<<"recv file finish. cliId:"<<cliId
                        <<"commId:"<<commId
                        <<"uuid:"<<GetClientData(cliId)->uuid
                        <<QTime::currentTime();

                //发送给通讯模块外部
                CommonMessageCode msg;
                msg.code = COMM_Code::CMD_FileSendFinish;
                CommonTimeInfo t;
                UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
                sprintf(msg.content, "对服务器的文件发送完毕. IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                        GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                        cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                        t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
                MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);
            }
                break;

            case CommandState::FileSendError:
            {
                //1.复归所有数据
                InitFileRecvProc(&GetClientData(cliId)->commData[commId].cmdData.fileRecv);
                GetClientData(cliId)->commData[commId].cmdData.curCmd = CommandState::Idle;

                qDebug()<<"recv file error! cliId:"<<cliId
                        <<"commId:"<<commId
                        <<"uuid:"<<GetClientData(cliId)->uuid
                        <<QTime::currentTime();

                //发送给通讯模块外部
                CommonMessageCode msg;
                msg.code = COMM_Code::CMD_FileSendError;
                CommonTimeInfo t;
                UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
                sprintf(msg.content, "对服务器的文件发送出现错误! IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                        GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                        cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                        t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
                MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);
            }
                break;

            default:
                break;
            }
        }
        else {
            //1,转发
            SendByDestId(v->destId, frame, frameLen);
        }
    }
        break;
    default:
        break;
    }

    return TRUE;
}

int BaseNetService::CheckFileRecvProcState(CMD_FILERECV *fileRecv, int state)
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

void BaseNetService::InitFileRecvProc(CMD_FILERECV *fileRecv)
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

void BaseNetService::StopFileRecvProc(int cliId, int commId)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) {
        return;
    }

    //1.向发送方发送接收错误帧,终止传输过程
    SendFileRecvState(cliId, commId, CommandState::FileRecvError);

    //2.初始化文件接收流程
    InitFileRecvProc(&GetClientData(cliId)->commData[commId].cmdData.fileRecv);
}

int BaseNetService::SendFileRecvState(int cliId, int commId, int state)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) {
        return FALSE;
    }

    FRM_FileTransState v;
    v.destId = GetClientData(cliId)->commData[commId].cmdData.fileRecv.srcId;
    v.state = state;

    int ret = SendFrame(cliId, commId, COMM_FRAMETYPE_FileTransState, sizeof(FRM_FileTransState), (unsigned char*)&v);

    return ret;
}

int BaseNetService::DealCallFileDir(int cliId, int commId, FRM_CallFileDir *v)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM || v == NULL) {
        return FALSE;
    }

    char path[COMMON_STRLEN_512];
    char text[128];
    UtilFunc::GetApplicationDirPath(path);

    sprintf(text, "%s", NETSRV_FileProc_FilePath);
    strcat_s(path, COMMON_STRLEN_512, text);

    FRM_CallFileDirRet fileDirRet;
    UtilFunc::GetFileDirInfo(path, fileDirRet);

    fileDirRet.uuid = v->uuid;

    SendFrame(cliId, commId, COMM_FRAMETYPE_CallNetSrvFileDirRet, sizeof(FRM_CallFileDirRet), (unsigned char*)&fileDirRet);

    return TRUE;
}

int BaseNetService::DealDownFile(int cliId, int commId, FRM_DownFile *v)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM || v == NULL) {
        return FALSE;
    }

    char path[COMMON_STRLEN_512];
    char text[128];
    UtilFunc::GetApplicationDirPath(path);

    sprintf(text, "%s/%s", NETSRV_FileProc_FilePath, v->fileName);
    strcat_s(path, COMMON_STRLEN_512, text);

    int srcId = 0;
    MakeAddressId(GetServerData()->uuid, GetServerData()->exeId, srcId);

    int destId = 0;
    MakeAddressId(GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, destId);

    int ret = SendFile(destId, srcId, path);
    return ret;
}

void BaseNetService::CommandProc(int cliId, int commId)
{
    if (cliId < 0 || cliId >= COMM_CLI_NUM || commId < 0 || commId >= COMM_CLICOMM_NUM) {
        return;
    }

    switch (GetClientData(cliId)->commData[commId].cmdData.curCmd) {
    case CommandState::Idle:
    {
        //do nothing
    }
        break;
    case CommandState::FileSend: //文件传输
    {
        FileSendProc(cliId, commId);
    }
        break;

    default:
        break;
    }
}

int BaseNetService::SendFile(int destId, int srcId, char *fullFilePath)
{
    if (fullFilePath == NULL) {
        return FALSE;
    }

    int uuid = -1, exeId = -1;
    ParseAddressId(destId, uuid, exeId);

    int findFlag = FALSE, pCliId = -1, pCommId = -1;

    //根据uuid和exeId查找通道
    for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
        if (GetClientData(cliId)->enableFlag == FALSE) {
            continue;
        }

        for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
            if (GetClientData(cliId)->commData[commId].connFlag == FALSE) {
                continue;
            }

            if (GetClientData(cliId)->commData[commId].exeId == exeId) {
                findFlag = TRUE;
                pCliId = cliId;
                pCommId = commId;
                break;
            }
        }
    }

    if (findFlag == FALSE || pCliId < 0 || pCliId >= COMM_CLI_NUM || pCommId < 0 || pCommId >= COMM_CLICOMM_NUM) {
        return FALSE;
    }

    //check
    if (GetClientData(pCliId)->commData[pCommId].cmdData.curCmd != CommandState::Idle) {
        return FALSE;
    }

    //赋值
    GetClientData(pCliId)->commData[pCommId].cmdData.curCmd = CommandState::FileSend;
    CMD_FILESEND *fileSend  = &GetClientData(pCliId)->commData[pCommId].cmdData.fileSend;
    fileSend->destId = destId;
    fileSend->sendInter = COMM_READPROC_INTER;
    fileSend->srcId  = srcId;
    fileSend->state  = CommandState::FileSendInit;
    fileSend->lastSendTime = UtilFunc::GetSysTimeMS();
    sprintf(fileSend->fullFilePath, "%s", fullFilePath);

    //发送给通讯模块外部
    CommonMessageCode msg;
    msg.code = COMM_Code::CMD_FileSendInit;
    CommonTimeInfo t;
    UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
    sprintf(msg.content, "服务器对外发送文件流程初始化. IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
            GetClientData(pCliId)->commData[pCommId].ipAddr, GetClientData(pCliId)->commData[pCommId].socket, \
            pCliId, pCommId, GetClientData(pCliId)->uuid, GetClientData(pCliId)->commData[pCommId].exeId, \
            t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
    MessageFrame(pCliId, pCommId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);

    return TRUE;
}

int BaseNetService::FileSendProc(int cliId, int commId)
{
    CMD_FILESEND *fileSend = &GetClientData(cliId)->commData[commId].cmdData.fileSend;

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

        SendFrame(cliId, commId, COMM_FRAMETYPE_FileSendRequest, sizeof(FRM_FileSendReq), (unsigned char*)&v);

        //命令状态置位
        fileSend->state = CommandState::FileSendSending;

        qDebug()<<"file send request"<<QTime::currentTime();

        //发送给通讯模块外部
        CommonMessageCode msg;
        msg.code = COMM_Code::CMD_FileSendRequest;
        CommonTimeInfo t;
        UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
        sprintf(msg.content, "服务器对外发送文件请求. IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
        MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);

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

        SendFrame(cliId, commId, COMM_FRAMETYPE_FileSendContent, sizeof(FRM_FileSendContent), (unsigned char*)&v);

        //命令状态置位---发送已完成
        if (finishFlag) {
            fileSend->state = CommandState::FileSendFinish;
        }
        qDebug()<<"file send content"<<QTime::currentTime();

        //发送给通讯模块外部
        MessageCode msg;
        msg.code = COMM_Code::CMD_FileSendSending;

        MessageFrame(cliId, commId, COMM_FRAMETYPE_MessageCode, sizeof(MessageCode), (unsigned char *)&msg);
    }
        break;

    case CommandState::FileSendFinish:
    {
        //1.向对端发送文件传输结束帧
        SendFileSendState(cliId, commId, CommandState::FileSendFinish);

        //2.复归所有数据(传输过程数据初始化)
        InitFileSendProc(cliId, commId);
        GetClientData(cliId)->commData[commId].cmdData.curCmd = CommandState::Idle;
        qDebug()<<"file send finish"<<QTime::currentTime();

        //发送给通讯模块外部
        CommonMessageCode msg;
        msg.code = COMM_Code::CMD_FileSendFinish;
        CommonTimeInfo t;
        UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
        sprintf(msg.content, "服务器对外发送文件完毕. IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
        MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);

    }
        break;

    case CommandState::FileSendError:
    {
        //1,复归所有数据
        InitFileSendProc(cliId, commId);
        GetClientData(cliId)->commData[commId].cmdData.curCmd = CommandState::Idle;
        qDebug()<<"file send error"<<QTime::currentTime();

        //2.向对端发送命令执行错误帧
        SendFileSendState(cliId, commId, CommandState::FileSendError);

        //发送给通讯模块外部
        CommonMessageCode msg;
        msg.code = COMM_Code::CMD_FileSendError;
        CommonTimeInfo t;
        UtilFunc::GetCurTimeInfo(t, UtilFunc::GetSysTimeMS());
        sprintf(msg.content, "对服务器的文件发送出现错误! IP:%s socket:%d cliId:%d commId:%d uuid:%d exeId:%d    时间:%04d-%02d-%02d %02d:%02d:%02d.%03d", \
                GetClientData(cliId)->commData[commId].ipAddr, GetClientData(cliId)->commData[commId].socket, \
                cliId, commId, GetClientData(cliId)->uuid, GetClientData(cliId)->commData[commId].exeId, \
                t.year, t.month, t.day, t.hour, t.minute, t.second, t.microSec);
        MessageFrame(cliId, commId, COMM_FRAMETYPE_CommonMessageCode, sizeof(CommonMessageCode), (unsigned char *)&msg);
    }
        break;

    default:

        break;
    }

    return TRUE;
}

void BaseNetService::InitFileSendProc(int cliId, int commId)
{
    CMD_FILESEND *fileSend = &GetClientData(cliId)->commData[commId].cmdData.fileSend;

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

int BaseNetService::CheckFileSendProcState(int cliId, int commId, int state)
{
    CMD_FILESEND *fileSend = &GetClientData(cliId)->commData[commId].cmdData.fileSend;

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

void BaseNetService::StopFileSendProc(int cliId, int commId)
{
    //1.向发送方发送接收错误帧,终止传输过程
    SendFileSendState(cliId, commId, CommandState::FileSendError);

    //2.初始化文件接收流程
    InitFileSendProc(cliId, commId);
}

int BaseNetService::SendFileSendState(int cliId, int commId, int state)
{
    CMD_FILESEND *fileSend = &GetClientData(cliId)->commData[commId].cmdData.fileSend;

    FRM_FileTransState v;
    v.destId = fileSend->destId;
    v.srcId  = fileSend->srcId;
    v.state = state;

    SendFrame(cliId, commId, COMM_FRAMETYPE_FileTransState, sizeof(FRM_FileTransState), (unsigned char *)&v);

    return TRUE;
}

int BaseNetService::PutTxBufByExeId(int exeId, unsigned char *buffer, int bufferLen)
{
    if (bufferLen <= 0 || buffer == NULL) return FALSE;

    for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
        if (GetClientData(cliId)->enableFlag == FALSE) {
            continue;
        }

        for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
            if (GetClientData(cliId)->commData[commId].connFlag == FALSE) {
                continue;
            }

            if (GetClientData(cliId)->commData[commId].exeId == exeId) {
                PutTxBuf(cliId, commId, buffer, bufferLen);
            }
        }
    }

    return TRUE;
}

int BaseNetService::SendByDestId(int destId, unsigned char *buffer, int bufferLen)
{
    if (bufferLen <= 0 || buffer == NULL) {
        return FALSE;
    }

    int destUuid = 0, destExeId = 0;

    ParseAddressId(destId, destUuid, destExeId);

    switch (destUuid) {
    //所有设备
    case Uuid::All:
    {
        switch (destExeId) {
        //所有exe
        case ExeId::AllExe:
        {
            PutSrvForwardingBufByRoomId(0, buffer, bufferLen);
        }
            break;
        //部分exe
        case ExeId::PartForwarding:
        {
            //待修改
            PutSrvForwardingBufByRoomId(0, buffer, bufferLen);
        }
            break;
        //指定某一类exe
        default:
        {
            PutTxBufByExeId(destExeId, buffer, bufferLen);
        }
            break;
        }
    }
        break;

    //指定设备
    default:
    {
        switch (destExeId) {
        //所有exe
        case ExeId::AllExe:
        {
            //目前没这种需求
        }
            break;
        //部分exe
        case ExeId::PartForwarding:
        {
            //目前没这种需求
        }
            break;
        //指定exe
        default:
        {
            for (int cliId = 0; cliId < COMM_CLI_NUM; cliId++) {
                if (GetClientData(cliId)->enableFlag == FALSE) {
                    continue;
                }

                if (GetClientData(cliId)->uuid == destUuid) {
                    for (int commId = 0; commId < COMM_CLICOMM_NUM; commId++) {
                        if (GetClientData(cliId)->commData[commId].connFlag == FALSE) {
                            continue;
                        }

                        if (GetClientData(cliId)->commData[commId].exeId == destExeId) {
                            PutTxBuf(cliId, commId, buffer, bufferLen);
                        }
                    }
                }
            }
        }
            break;
        }
    }
        break;
    }

    return TRUE;
}

void BaseNetService::ParseAddressId(int addrId, int &uuid, int &exeId)
{
    uuid = addrId/10000;
    exeId = addrId%10000;
}

void BaseNetService::MakeAddressId(int uuid, int exeId, int &addrId)
{
    addrId = uuid*10000+exeId;
}
