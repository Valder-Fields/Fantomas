

#include "communicator.h"

extern int glo_liveflag;
extern int glo_threadnum;

Communicator::Communicator()
{
    m_service = NULL;
}

Communicator::~Communicator()
{

}

int Communicator::Start(int protoType)
{
    int ret = FALSE;
    ret = Init(protoType);
    if (ret == FALSE) {
        return ret;
    }

    m_service = new CommService(this);
    ret = m_service->StartCommProc();
    if (ret == FALSE) {
        return ret;
    }

    return TRUE;
}

int Communicator::Stop()
{
    if (m_service) {
        m_service->StopCommProc();

        delete m_service;
        m_service = NULL;
    }


    return TRUE;
}

int Communicator::GetUuid()
{
    return CommManager::GetUuid();
}

int Communicator::GetExeId()
{
    return CommManager::GetExeId();
}

int Communicator::SendFrame(int type, int length, unsigned char *value)
{
    return CommManager::SendFrame(type, length, value);
}

int Communicator::SendFile(int destId, int srcId, char *fullFilePath)
{
    return CommManager::SendFile(destId, srcId, fullFilePath);
}

