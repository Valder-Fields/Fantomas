

#include "scommunicator.h"

extern int glo_liveflag;
extern int glo_threadnum;

SCommunicator::SCommunicator()
{
    m_service = NULL;
}

SCommunicator::~SCommunicator()
{

}

int SCommunicator::Start(int protoType)
{
    int ret = FALSE;
    ret = Init(protoType);
    if (ret == FALSE) {
        return ret;
    }

    m_service = new SCommService(this);
    ret = m_service->StartCommProc();
    if (ret == FALSE) {
        return ret;
    }

    return TRUE;
}

int SCommunicator::Stop()
{
    if (m_service) {
        m_service->StopCommProc();

        delete m_service;
        m_service = NULL;
    }

    return TRUE;
}

int SCommunicator::SendFrame(int cliId, int commId, int type, int length, unsigned char *value)
{
    return SCommManager::SendFrame(cliId, commId, type, length, value);
}

int SCommunicator::MakeAWholeFrame(int type, unsigned char *value, int valueLen, unsigned char *frame, int &frameLen)
{
    return SCommManager::MakeAWholeFrame(type, value, valueLen, frame, frameLen);
}


