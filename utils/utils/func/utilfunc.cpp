


#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

#ifdef _WIN32
#include <windows.h>
#define EPOCHFILETIME   (116444736000000000UL)
#endif



#include "utilfunc.h"



UtilFunc::UtilFunc()
{
 
}

UtilFunc::~UtilFunc()
{

}

void UtilFunc::GetApplicationDirPath(char *path)
{
    if (path == NULL) {
        return;
    }

    sprintf(path, "%s", QCoreApplication::applicationDirPath().toLocal8Bit().constData());
}

int64_t UtilFunc::GetSysTimeMS()
{
#ifdef EPOCHFILETIME
    FILETIME ft;
    LARGE_INTEGER li;
    int64_t tt = 0;
    GetSystemTimeAsFileTime(&ft);
    li.LowPart = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    //从1970
    tt = (li.QuadPart - EPOCHFILETIME) / 10;
    return tt/1000;
#else
    timeval tv;
    gettimeofday(&tv, 0);
    return (int64_t)tv.tv_sec *1000000 + (int64_t)tv.tv_used;
#endif

    return 0;

}

void UtilFunc::GetCurTimeInfo(CommonTimeInfo &t, const int64_t &time_ms)
{
    int64_t time = time_ms/1000;
    tm tmpTime;
    localtime_s(&tmpTime, &time);

    t.year   = tmpTime.tm_year+1900;
    t.month  = tmpTime.tm_mon+1;
    t.day    = tmpTime.tm_mday;
    t.hour   = tmpTime.tm_hour;
    t.minute = tmpTime.tm_min;
    t.second = tmpTime.tm_sec;
    t.microSec = time_ms%1000;
}

int UtilFunc::GetFileLength(FILE *fp, const char *fileName, int &fileLength)
{
    if (fileName == NULL) {
        return FALSE;
    }

    errno_t err;
    err = fopen_s(&fp, fileName, "rb");
    if (err != 0) {
        return FALSE;
    }

    fseek(fp, 0, SEEK_END);

    fileLength = ftell(fp);

    fclose(fp);

    return TRUE;
}

bool UtilFunc::CreateDirPath(const char *dir)
{
    if (dir == NULL) {
        return FALSE;
    }

    QDir targetDir;
    bool createFlag = false;
    bool exist = targetDir.exists(QString::fromLocal8Bit(dir));
    if (exist == false) {
        createFlag = targetDir.mkpath(QString::fromLocal8Bit(dir));
    }
    else {
        createFlag = true;
    }

    return createFlag;
}

int UtilFunc::GetFileNameByFullFilePath(const char *fullFilePath, char *fileName)
{
    if (fullFilePath == NULL || fileName == NULL) {
        return FALSE;
    }

    QFileInfo file;
    file = QFileInfo(QString::fromLocal8Bit(fullFilePath));

    sprintf(fileName, "%s",  file.fileName().toLocal8Bit().constData());

    return TRUE;
}

int UtilFunc::ReadFile(FILE *fp, const char *fileName, unsigned char *buffer, int bufferLen)
{
    if (fileName == NULL || buffer == NULL || bufferLen <= 0) {
        return FALSE;
    }

    int ret = 0;
    errno_t err;
    err = fopen_s(&fp, fileName, "rb");
    if (err != 0) {
        return FALSE;
    }

    ret = (int)fread(buffer, bufferLen, 1, fp);
    if (ret != 1) {
        qDebug()<<"ReadFile fread error!";
    }

    fclose(fp);
    fp = NULL;

    return TRUE;
}

bool UtilFunc::DelFile(const char *fileName)
{
    return QFile::remove(fileName);
}

int UtilFunc::SaveFile(const char *fileName, char *buffer, int bufferLen, int mode)
{
    if (fileName == NULL || buffer == NULL || bufferLen <= 0) {
        return FALSE;
    }

    FILE *saveFp = NULL;
    errno_t err = 0;
    switch (mode) {
    case FILEOPER_MODE_APPEND:
    {
        err = fopen_s(&saveFp, fileName, "ab+");
    }
        break;

    case FILEOPER_MODE_NEW:
    {
        err = fopen_s(&saveFp, fileName, "wb+");
    }
        break;

    default:
        break;
    }

    if (err != 0) {
        return FALSE;
    }

    fwrite(buffer, bufferLen, 1, saveFp);
    fclose(saveFp);
    saveFp = NULL;

    return TRUE;
}

int UtilFunc::GetFileDirInfo(const char *path, FRM_CallFileDirRet &v)
{
    if (path == NULL) {
        return FALSE;
    }

    QDir dir(path);
    QStringList nameFilters;
    //nameFilters << "*.jpg" << "*.png";
    QStringList files = dir.entryList(nameFilters, QDir::Files|QDir::Readable, QDir::Name);

    v.fileCount = files.size();
    for (int i = 0; i < files.size(); i++) {
        if (i >= 16) {
            break;
        }

        memcpy(v.fileNameGroup+i*256, files[i].toLocal8Bit().constData(), 256);
    }

    return TRUE;
}

int UtilFunc::PrintHexStr(char *str, int size)
{
    QString tmpstr;
    QString tmpchar;
    char tmp[4];
    for (int i = 0; i < size; i++) {
        sprintf(tmp, "%02x ", (unsigned char)str[i]);
        tmpchar = QString::fromLocal8Bit("%1 ").arg((short)str[i], 2, 16, QChar('0'));
        tmpstr += tmp;
    }
    qDebug()<<tmpstr;

    return TRUE;
}

int UtilFunc::PrintDexStr(char *str, int size)
{
    QString tmpstr;
    char tmp[6];
    for (int i = 0; i < size; i++) {
        sprintf(tmp, "%03d ", (unsigned char)str[i]);
        tmpstr += tmp;
    }
    qDebug()<<tmpstr;

    return TRUE;
}
