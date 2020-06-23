


#include "protocol_simpletlv.h"

ProtoSimpleTlv::ProtoSimpleTlv()
{

}

ProtoSimpleTlv::~ProtoSimpleTlv()
{

}

int ProtoSimpleTlv::GetAWholeFrame(COMM_CHANNEL_RXBUF *rxbuf, int &type, unsigned char *frame, int &frameLen)
{
    if (rxbuf == NULL || frame == NULL) {
        return FALSE;
    }

    //1.解析一帧
    int pType = -1, ret = -1, valueLen = 0, unParseLen = 0;

    if (rxbuf->head == rxbuf->tail) {
        return SIMPLETLV_WARNING_HeadEqualTail;
    }

    //异常检查:剩余字节<T+L
    unParseLen = (rxbuf->tail+COMM_RXBUF_LEN-rxbuf->head)%COMM_RXBUF_LEN;
    if (unParseLen < (SIMPLETLV_Frame_Type+SIMPLETLV_Frame_Length)) {
        //qDebug()<<"ProtoSimpleTlv:SIMPLETLV_WARNING_LastLenLessThanTL !!!!!!!!!!!!"<<QTime::currentTime();
        QDEBUG("ProtoSimpleTlv warning: -100");
        return SIMPLETLV_WARNING_LastLenLessThanTL;
    }

    pType    = ParseFrameType(rxbuf);
    valueLen = ParseFrameLenth(rxbuf);

    //异常检查:剩余字节<T+L+V(非本地处理业务都可以直接转发,不进行本判断;只有需要本地处理的业务,才需要一个完整的帧进行处理)
    if (unParseLen < (SIMPLETLV_Frame_Type+SIMPLETLV_Frame_Length+valueLen)) {
        //qDebug()<<"ParseCliRxbuf: unParseLen...2 !!!!!!!!!!!! type:"<<type<<"unParseLen:"<<unParseLen<<"frameLen:"<<(SIMPLETLV_Frame_Type+SIMPLETLV_Frame_Length+valueLen)<<"tail:"<<rxbuf->tail<<"head:"<<rxbuf->head;
        QDEBUG("ProtoSimpleTlv warning: -101");
        return SIMPLETLV_WARNING_LastLenLessThanTLV;
    }

    frameLen = valueLen+SIMPLETLV_Frame_Type+SIMPLETLV_Frame_Length;

    //异常检查:检查帧长度
    if (frameLen > COMM_FRAME_MAXNUM) {
        //qDebug()<<"ParseCliRxbuf: frameLen > NETSRV_FRAME_MAXNUM !!!!!!!!!!!!";
        QDEBUG("ProtoSimpleTlv warning: -200");
    }

    //2.从接收缓存取出完整的一帧传出
    ret = GetCommRxbuf(rxbuf, frame, frameLen);
    if (ret == FALSE) {
        return FALSE;
    }

    type = pType;

    return TRUE;
}

int ProtoSimpleTlv::MakeAWholeFrame(int type, unsigned char *value, int valueLen, unsigned char *frame, int &frameLen)
{
    if (valueLen < 0 || valueLen > COMM_FRAME_MAXNUM) {
        return SIMPLETLV_WARNING_SendFrameLenOverLimit;
    }

    if (value == NULL || frame == NULL) {
        return FALSE;
    }

    frameLen = valueLen+SIMPLETLV_Frame_Type+SIMPLETLV_Frame_Length;

    //T
    frame[0] = type%256;
    frame[1] = type/256;

    //L
    frame[2] = valueLen%256;
    frame[3] = (valueLen/256)%256;
    frame[4] = (valueLen/(256*256))%256;
    frame[5] = (valueLen/(256*256*256))%256;

    //V
    memcpy((frame+SIMPLETLV_Frame_Type+SIMPLETLV_Frame_Length), value, valueLen);

    return TRUE;
}

int ProtoSimpleTlv::ParseFrameType(COMM_CHANNEL_RXBUF *rxbuf, int typeLen)
{
    (void)typeLen;
    if (rxbuf == NULL) {
        return FALSE;
    }

    unsigned char val1, val2;
    int idx = rxbuf->head;
    val1 = rxbuf->buf[idx];
    val2 = rxbuf->buf[(idx+1)%COMM_RXBUF_LEN];

    return (val2*256 + val1);
}

int ProtoSimpleTlv::GetValueByFrame(unsigned char *frame, int frameLen, unsigned char **value, int &valueLen)
{
    if (frame == NULL || frameLen < 0) {
        return FALSE;
    }

    *value = (unsigned char *)(frame+SIMPLETLV_Frame_Type+SIMPLETLV_Frame_Length);
    valueLen = frameLen-SIMPLETLV_Frame_Type-SIMPLETLV_Frame_Length;

    return TRUE;
}

int ProtoSimpleTlv::ParseFrameLenth(COMM_CHANNEL_RXBUF *rxbuf, int lenLen)
{
    (void)lenLen;
    if (rxbuf == NULL) {
        return FALSE;
    }

    unsigned char val1, val2, val3, val4;
    int idx = rxbuf->head+SIMPLETLV_Frame_Type;

    val1 = rxbuf->buf[(idx)%COMM_RXBUF_LEN];
    val2 = rxbuf->buf[(idx+1)%COMM_RXBUF_LEN];
    val3 = rxbuf->buf[(idx+2)%COMM_RXBUF_LEN];
    val4 = rxbuf->buf[(idx+3)%COMM_RXBUF_LEN];

    return (val4*256*256*256+val3*256*256+val2*256+val1);
}

int ProtoSimpleTlv::GetCommRxbuf(COMM_CHANNEL_RXBUF *rxbuf, unsigned char *buffer, int bufferLen)
{
    if (rxbuf == NULL || buffer == NULL) {
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
