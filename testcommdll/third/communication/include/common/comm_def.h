/********************************************************************************************************
*                                     communication                                                     *
*                                                                                                       *
*                                   (c) Copyright 2020                                                  *
*                                  All Rights Reserved                                                  *
*                                                                                                       *
*   FileName    :   comm_def.h                                                                          *
*   Description :   通讯数据结构定义                                                                       *
*   Author      :   Valder-Fields                                                                                 *
*   Date        :                                                                                       *
*   Remark      :                                                                                       *
*                                                                                                       *
*   No.         Date         Modifier        Description                                                *
*   -----------------------------------------------------------------------------------------------     *
*********************************************************************************************************/

#ifndef __STRUCT_DEF_H__
#define __STRUCT_DEF_H__

#include "../../../common/def/common_def.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 通信
////////////////////////////////////////////////////////////////////////////////////////////////////////
#define     COMM_SENDDING_LENTH           (1024*1024*1)   //socket单次发送缓存长度(稳定版为3M)

#define     COMM_RXBUF_LEN                (1024*1024*2)   //客户端接收缓存大小(10M) //todo //test :稳定版为5M
#define     COMM_TXBUF_LEN                (1024*1024*2)   //客户端发送缓存大小(10M) //todo //test :稳定版为5M

#define     COMM_SRVTXBUF_LEN             (1024*1000*3)   //服务端发送缓存大小(20M) //todo 为测试play流程快速覆盖循环数组,测试循环数组逻辑,目前调小一些

#define     COMM_SINGLE_READ_SIZE         8192            //通道单次读取长度
#define     COMM_FRAME_MAXNUM             81920           //业务接收单帧最大长度

//协议层
#define     COMM_PROTOCOL_SimpleTlv       10000           //简单TLV协议

//通讯层帧类型
#define     COMM_FRAMETYPE_Heartbeat      1001            //心跳帧
#define     COMM_FRAMETYPE_MessageCode    1002            //简单消息码帧
#define     COMM_FRAMETYPE_CommonMessageCode    1003            //通用消息码帧

//业务帧类型
#define     COMM_FRAMETYPE_CommTest       2000            //通讯测试
#define     COMM_FRAMETYPE_LoginReq       2001            //客户端登录
#define     COMM_FRAMETYPE_LoginAck       2002            //客户端登录确认
#define     COMM_FRAMETYPE_ClientState    2003            //客户端登录状态

#define     COMM_FRAMETYPE_SControlTrainRet   3001        //训练控制命令回执

#define     COMM_FRAMETYPE_FileSendRequest    4001        //文件传输请求
#define     COMM_FRAMETYPE_FileSendContent    4002        //文件传输内容
#define     COMM_FRAMETYPE_FileTransState     4003        //文件传输状态(结束,错误等)


//导控端帧类型
#define     COMM_FRAMETYPE_SendSet        33101           //下发想定
#define     COMM_FRAMETYPE_ControlTrain   31201           //控制命令(开始、暂停...)
#define     COMM_FRAMETYPE_PlaybackRate   31202           //快进倍数(回放)

//U3D帧类型
#define     COMM_FRAMETYPE_SendSetRet        42002           //训练内容初始化完成
#define     COMM_FRAMETYPE_ControlTrainRet   42003           //控制命令回执

//SCADA
//客户端帧类型
#define     COMM_FRAMETYPE_ClientSendAllData    8001           //客户端数据
#define     COMM_FRAMETYPE_CallNetSrvFileDir    8100           //召唤服务器文件目录
#define     COMM_FRAMETYPE_CallNetSrvFileDirRet 8200           //召唤服务器文件目录返回
#define     COMM_FRAMETYPE_DownFile             8101           //召唤服务器文件

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 服务
////////////////////////////////////////////////////////////////////////////////////////////////////////

//client
#define     COMM_READPROC_INTER           15      //读线程轮循间隔(ms)
#define     COMM_DISPATCHPROC_INTER       15      //解析线程轮循间隔(ms)
#define     COMM_FORWARDING_INTER         15      //转发线程轮循间隔(ms)
//#define     COMM_SAVEPROC_INTER           30    //保存线程轮循间隔(ms)
#define     COMM_TRANSACTIONPROC_INTER    500     //事务处理线程轮循间隔(ms)

#define     COMM_CLICOMM_NUM              12       //客户端通讯通道个数

#define     COMM_CLI_NUM                  15      //系统客户端最大个数
//#define     COMM_LISTEN_MAXNUM            10      //服务器监听个数(端口与程序映射最大个数)

#define     COMM_SRVCOMM_NUM              1       //服务器监听端口个数
#define     COMM_SRVROOM_NUM              1       //test 目前为:COMM_CLI_NUM 服务器房间最大个数(应等于同时训练的最大课程数,目前="客户端最大个数")

//server
#define     COMM_SRVREADPROC_INTER           3      //读线程轮循间隔(ms)
#define     COMM_SRVDISPATCHPROC_INTER       2      //解析线程轮循间隔(ms)
#define     COMM_SRVFORWARDING_INTER         2      //转发线程轮循间隔(ms)
#define     COMM_SRVACCEPTPROC_INTER         500     //监听线程轮循间隔(ms)
//#define     COMM_SAVEPROC_INTER           30    //保存线程轮循间隔(ms)
#define     COMM_SRVTRANSACTIONPROC_INTER    500     //事务处理线程轮循间隔(ms)

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 业务
////////////////////////////////////////////////////////////////////////////////////////////////////////

//文件收发
#define     NETSRV_FileProc_FilePath         "/data/"      //传输文件存放路径

//存储
#define     NETSRV_SRVSAVE_FILEPATH         "/procdb/"      //文件存储和回放相对路径

#define     NETSRV_SRVSAVE_RCDINFONUM       1000            //保存索引个数
#define     NETSRV_SRVSAVE_BUFLENTH         (1024*1000)     //单次保存最大缓存(1M,待提高)

#define     NETSRV_SRVSAVE_BYTES_ID         4       //ID所用字节数
#define     NETSRV_SRVSAVE_BYTES_LENTH      4       //长度所用字节数

//回放
#define     NETSRV_SRVPLAY_INFONUM          1000    //回放记录缓存
//#define     NETSRV_SRVPLAY_BUFLENTH         NETSRV_SRVSAVE_BUFLENTH   //单次读取最大缓存
#define     NETSRV_SRVPLAY_BUFLEN           (1024*1000*20) //方案2回放缓存

#define     NETSRV_SRVPLAY_FILELEN_LMT      (1024*1000*1000) //文件大小界限(1000M)
#define     NETSRV_SRVPLAY_MODE_READALL     1       //一次性读取全部内容
#define     NETSRV_SRVPLAY_MODE_BUFF        2       //缓存方式

#define     NETSRV_SRVPLAY_RCD_MAXNUM       2000000  //数据记录索引的最大条数

#define     NETSRV_SRVPLAY_RATE_LIMIT       8       //倍速限制(8倍速)

#define     NETSRV_SRVPLAY_PARSE_RUNNING    1       //回放解析进行
#define     NETSRV_SRVPLAY_PARSE_FINISH     2       //回放解析完毕
#define     NETSRV_SRVPLAY_PARSE_ERROR      3       //回放解析发生错误
#define     NETSRV_SRVPLAY_PARSE_WAIT       4       //回放时解析等待

#define     NETSRV_SRVPLAY_SEND_RUNNING     1       //回放转发进行
#define     NETSRV_SRVPLAY_SEND_FINISH      2       //回放转发结束
#define     NETSRV_SRVPLAY_SEND_ERROR       3       //回放转发错误

#define     NETSRV_SRVPLAY_CTTYPE_STATE     1       //控制类型-播放状态
#define     NETSRV_SRVPLAY_CTTYPE_SPEED     2       //控制类型-速度控制

//文件发送
#define     CMD_FILESEND_LENTH_LIMIT           (512*1024*1024) //文件大小界限(512M)
#define     CMD_FILERECV_LENTH_LIMIT           (512*1024*1024) //文件大小界限(512M)
#define     CMD_FILESEND_SINGLE_LENTH            4096        //文件单次发送字节数

//错误码(提供给外部使用)
#define     NETSRV_ERR_SAVE_Common          -2000   //保存过程-普通错误
#define     NETSRV_ERR_PLAY_Common          -3000   //回放过程-普通错误


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 业务类型值定义
////////////////////////////////////////////////////////////////////////////////////////////////////////
enum Uuid {
    All = 0,                //所有设备
};

enum ExeId {
    AllExe = 0,             //代表所有exe
    SysController,          //系统管理控制端
    SysClient,              //系统管理客户端
    NetServer,              //系统联网服务器
    Trainer = 10,           //三维训练端
    Effector,               //效应器
    TrainController = 20,   //训练导控端
    PartForwarding = 9999,  //局部转发
};

enum ForwardingMode {
    ServerControl = 0,      //联网服务器控制转发
    UserControl,            //用户控制转发
};

//通讯模块输出信息-连接
enum COMM_Code {
    ConnectSuccess = 100,      //连接成功
    ConnectFail    = 101,      //连接失败
    ConnectBreak   = 102,      //连接断开
    LoginSuccess   = 110,      //登录成功反馈
    LoginFail      = 111,      //登录失败反馈

    CMD_FileSendInit    = 201,     //文件发送初始化
    CMD_FileSendRequest = 202,     //文件发送请求
    CMD_FileSendSending = 203,     //文件发送内容
    CMD_FileSendFinish  = 204,     //文件发送完成
    CMD_FileSendError   = 205,     //文件发送错误

    CMD_FileRecvRecving = 302,     //文件接收内容
    CMD_FileRecvFinish  = 303,     //文件接收完成
    CMD_FileRecvError   = 304,     //文件接收错误
};

//客户端登录
enum CLIENT_LoginAck {
    Allow = 1,          //通过
    Deny  = 2,          //拒绝
};

//客户端状态
enum CLIENT_State {
    Online  = 1,        //上线
    Offline = 2,        //下线
};

//训练类型
enum TrainType{
    Common = 1,         //普通训练
    Playback,           //回放
};

//训练控制
enum ControlTrain {
    Init        = 0,    //0-初态
    Start       = 1,    //1-开始
    Pause,              //2-暂停
    Continue,           //3-继续
    Stop,               //4-停止
    Error,              //5-错误
};

//控制命令:控制状态
enum CommandState {
    Idle                = 0,       //空闲
    FileSend            = 200,     //文件发送
    FileSendInit        = 201,     //文件发送-初始化
    FileSendSending,               //文件发送-传输中
    FileSendFinish,                //文件发送-传输完毕
    FileSendBreak,                 //文件发送-中断传输
    FileSendError       = 299,     //文件发送-错误

    FileRecv            = 300,     //文件接收
    FileRecvRecving,               //文件接收-接收中
    FileRecvFinish,                //文件接收-完毕
    FileRecvError,                 //文件接收-错误
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 帧定义
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct MessageCode
{
    int code;    //通讯模块信息代码
};

struct CommonMessageCode
{
    int code;    //通讯模块信息代码
    char content[COMMON_STRLEN_256]; //消息内容
};

struct Heartbeat
{
    int     exeId;
    char    state;
};

//客户端登录请求
struct FRM_LoginReq
{
    int uuid;
    int exeId;
};

//服务器登录确认
struct FRM_LoginAck
{
    int ret;	//1:通过请求 2:拒绝请求
};

//客户端状态
struct FRM_ClientState
{
    int uuid;
    int state;  //1:客户端上线 2:客户端下线
};

struct FRM_FileSendReq
{
    int    destId;     //发送目的地寻址ID
    int    srcId;      //发送源寻址ID
    int    fileLen;    //文件长度
    char   fileName[COMMON_STRLEN_256]; //文件名
};

struct FRM_FileSendContent
{
    int    destId;     //发送目的地寻址ID
    int    srcId;      //发送源寻址ID
    int    size;       //内容长度
    unsigned char content[CMD_FILESEND_SINGLE_LENTH]; //文件内容
};

//文件传输状态
struct FRM_FileTransState
{
    int    destId;     //发送目的地寻址ID
    int    srcId;      //发送源寻址ID
    int    state;      //文件传输结束状态位(正常结束/发生错误/控制终止)
};

struct FRM_CallFileDir
{
    int uuid;
};

struct FRM_CallFileDirRet
{
    int uuid;
    int fileCount;
    char fileNameGroup[16*256];
};

struct FRM_DownFile
{
    int uuid;
    char fileName[COMMON_STRLEN_256];
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 帧定义-导控端
////////////////////////////////////////////////////////////////////////////////////////////////////////
//下发想定帧
struct FRM_SendSet
{
    int  trainType;
    char name[100];
    char path[256];
};

//控制命令帧
struct FRM_ControlTrain{
    int  trainType;
    int  procedure; //1-开始 2-暂停 3-继续 4-停止 5-错误
};

//回放快进倍数帧
struct GL_FastForward{
    int rate; // 1—1X 2—2X 3—3X 4—4X
};

struct FRM_ControlTrainRet
{
    int uuid;           //实体计算机ID
    int exeId;          //exe的ID(U3D普通学员端:10, U3D效应器:11)
    int trainType;      //训练类型
    int trainState;     //训练状态
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 数据结构
////////////////////////////////////////////////////////////////////////////////////////////////////////

//communication
struct COMM_CHANNEL_RXBUF
{
    int     head;
    int     tail;

    int     onceFrameLen;                       //单次接收一帧长度
    unsigned char onceFrame[COMM_FRAME_MAXNUM]; //单次接收一帧数据
    unsigned char buf[COMM_RXBUF_LEN];
};

struct COMM_CHANNEL_TXBUF
{
    int     head;
    int     tail;

    int     onceBufLen;                         //单次发送一次数据长度
    unsigned char onceBuf[COMM_SENDDING_LENTH]; //单次发送一次数据
    unsigned char buf[COMM_TXBUF_LEN];
};

struct SRV_COMM_CHANNEL_TXBUF
{
    int     head;
    int     tail;

    int     onceBufLen;                   //单次发送数据长度
    char    onceBuf[COMM_SENDDING_LENTH]; //单次发送数据
    unsigned char buf[COMM_SRVTXBUF_LEN];    //服务器发送缓存大小(通常比客户端大)
};

//业务
//控制-文件发送
struct CMD_FILESEND
{
    //para
    int     destId;             //发送目的地寻址ID
    int     srcId;              //发送源寻址ID
    int     overtime;           //超时控制-发送超时时间(单位:s)
    int     sendInter;          //发送间隔(ms)
    char    fullFilePath[COMMON_STRLEN_512];    //发送文件全路径

    //data
    int     state;              //当前命令执行状态
    int64_t lastSendTime;       //上次发送时间(ms)
    int     sendIndex;          //发送文件指针索引
    int     fileLen;            //文件长度
    unsigned char * fileBuf;    //文件指针
};

//控制-文件接收
struct CMD_FILERECV
{
    //para
    int     srcId;              //发送源寻址ID
    int     overtime;           //超时控制-发送超时时间(单位:s)
    char    fullFilePath[COMMON_STRLEN_512];    //接收文件全路径

    //data
    int     state;              //当前命令执行状态
    int     recvIndex;          //接收文件指针索引
    int     fileLen;            //文件长度
    unsigned char * fileBuf;    //存放文件数据内存指针
};

//控制类数据
struct COMMAND_DATA
{
    int             curCmd;    //当前命令号
    CMD_FILESEND    fileSend;  //文件命令流程控制
    CMD_FILERECV    fileRecv;  //文件接收流程控制
};

//server
//server-rundata
struct NETSRV_RUNDATA
{
    //接收
    int     recvUnitFlow;       //接收瞬时流量(Bytes/s)
    int     recvPeakFlow;       //接收峰值流量(Bytes/s)
    int     recvAllFlow;        //接收数据总数(Bytes)
    int     lastRecvAllFlow;    //上一时间接收数据总数(Bytes)

    int     recvUnitFrame;      //接收瞬时帧数(帧/s)
    int     recvPeakFrame;      //接收峰值帧数(帧/s)
    int     recvAllFrame;       //接收总帧数(帧)
    int     lastRecvAllFrame;   //上一时间接收总帧数(帧)

    //发送
    int     sendUnitFlow;       //发送瞬时流量(Bytes/s)
    int     sendPeakFlow;       //发送峰值流量(Bytes/s)
    int     sendAllFlow;        //发送数据总数(Bytes)
    int     lastSendAllFlow;    //上一时间发送数据总数(Bytes)

    int     sendUnitFrame;      //发送瞬时帧数(帧/s)
    int     sendPeakFrame;      //发送峰值帧数(帧/s)
    int     sendAllFrame;       //发送总帧数(帧)
    int     lastSendAllFrame;   //上一时间接收总帧数(帧)

    //系统
    int     startTime;          //系统启动时间(s)
    int     sysRunTime;         //系统运行时间(s)

    int     forwardingCache1UnitVal;  //系统(房间0)转发缓存瞬时使用量(Bytes/s)
    int     forwardingCache1PeakVal;  //系统(房间0)转发缓存峰值使用量(Bytes/s)
    float   forwardingCache1UnitRate; //系统(房间0)转发缓存每秒瞬时利用率(%)
    float   forwardingCache1PeakRate; //系统(房间0)转发缓存每秒峰值利用率(%)

    //客户端
    int     Client1Comm1CacheUnitRate;//客户端1通道1缓存每秒瞬时利用率(%)
    int     Client1Comm1CachePeakRate;//客户端1通道1缓存每秒峰值利用率(%)
    int     Client1Comm2CacheUnitRate;//客户端1通道2缓存每秒瞬时利用率(%)
    int     Client1Comm2CachePeakRate;//客户端1通道2缓存每秒峰值利用率(%)
    int     Client1Comm3CacheUnitRate;//客户端1通道3缓存每秒瞬时利用率(%)
    int     Client1Comm3CachePeakRate;//客户端1通道3缓存每秒峰值利用率(%)
    int     Client1Comm4CacheUnitRate;//客户端1通道4缓存每秒瞬时利用率(%)
    int     Client1Comm4CachePeakRate;//客户端1通道4缓存每秒峰值利用率(%)
    int     Client1Comm5CacheUnitRate;//客户端1通道5缓存每秒瞬时利用率(%)
    int     Client1Comm5CachePeakRate;//客户端1通道5缓存每秒峰值利用率(%)
    int     Client1Comm6CacheUnitRate;//客户端1通道6缓存每秒瞬时利用率(%)
    int     Client1Comm6CachePeakRate;//客户端1通道6缓存每秒峰值利用率(%)

    int     Client2Comm1CacheUnitRate;//客户端2通道1缓存每秒瞬时利用率(%)
    int     Client2Comm1CachePeakRate;//客户端2通道1缓存每秒峰值利用率(%)
    int     Client2Comm2CacheUnitRate;//客户端2通道2缓存每秒瞬时利用率(%)
    int     Client2Comm2CachePeakRate;//客户端2通道2缓存每秒峰值利用率(%)
    int     Client2Comm3CacheUnitRate;//客户端2通道3缓存每秒瞬时利用率(%)
    int     Client2Comm3CachePeakRate;//客户端2通道3缓存每秒峰值利用率(%)
    int     Client2Comm4CacheUnitRate;//客户端2通道4缓存每秒瞬时利用率(%)
    int     Client2Comm4CachePeakRate;//客户端2通道4缓存每秒峰值利用率(%)
    int     Client2Comm5CacheUnitRate;//客户端2通道5缓存每秒瞬时利用率(%)
    int     Client2Comm5CachePeakRate;//客户端2通道5缓存每秒峰值利用率(%)
    int     Client2Comm6CacheUnitRate;//客户端2通道6缓存每秒瞬时利用率(%)
    int     Client2Comm6CachePeakRate;//客户端2通道6缓存每秒峰值利用率(%)

    int     Client3Comm1CacheUnitRate;//客户端3通道1缓存每秒瞬时利用率(%)
    int     Client3Comm1CachePeakRate;//客户端3通道1缓存每秒峰值利用率(%)
    int     Client3Comm2CacheUnitRate;//客户端3通道2缓存每秒瞬时利用率(%)
    int     Client3Comm2CachePeakRate;//客户端3通道2缓存每秒峰值利用率(%)
    int     Client3Comm3CacheUnitRate;//客户端3通道3缓存每秒瞬时利用率(%)
    int     Client3Comm3CachePeakRate;//客户端3通道3缓存每秒峰值利用率(%)
    int     Client3Comm4CacheUnitRate;//客户端3通道4缓存每秒瞬时利用率(%)
    int     Client3Comm4CachePeakRate;//客户端3通道4缓存每秒峰值利用率(%)
    int     Client3Comm5CacheUnitRate;//客户端3通道5缓存每秒瞬时利用率(%)
    int     Client3Comm5CachePeakRate;//客户端3通道5缓存每秒峰值利用率(%)
    int     Client3Comm6CacheUnitRate;//客户端3通道6缓存每秒瞬时利用率(%)
    int     Client3Comm6CachePeakRate;//客户端3通道6缓存每秒峰值利用率(%)

    int     Client4Comm1CacheUnitRate;//客户端4通道1缓存每秒瞬时利用率(%)
    int     Client4Comm1CachePeakRate;//客户端4通道1缓存每秒峰值利用率(%)
    int     Client4Comm2CacheUnitRate;//客户端4通道2缓存每秒瞬时利用率(%)
    int     Client4Comm2CachePeakRate;//客户端4通道2缓存每秒峰值利用率(%)
    int     Client4Comm3CacheUnitRate;//客户端4通道3缓存每秒瞬时利用率(%)
    int     Client4Comm3CachePeakRate;//客户端4通道3缓存每秒峰值利用率(%)
    int     Client4Comm4CacheUnitRate;//客户端4通道4缓存每秒瞬时利用率(%)
    int     Client4Comm4CachePeakRate;//客户端4通道4缓存每秒峰值利用率(%)
    int     Client4Comm5CacheUnitRate;//客户端4通道5缓存每秒瞬时利用率(%)
    int     Client4Comm5CachePeakRate;//客户端4通道5缓存每秒峰值利用率(%)
    int     Client4Comm6CacheUnitRate;//客户端4通道6缓存每秒瞬时利用率(%)
    int     Client4Comm6CachePeakRate;//客户端4通道6缓存每秒峰值利用率(%)
};

struct NETSRV_SYSDATA
{
    int     readProcLoopCount;      //读取线程轮循次数
    int     writeProcLoopCount;     //发送线程轮循次数
    int     dispatchProcLoopCount;  //dispatch线程轮循次数
    int     saveProcLoopCount;      //save线程轮循次数

    int     heartCheckCount;        //心跳检查计数
};

//server-comm
struct NETSRV_SRVCOMM
{
    SRV_COMM_CHANNEL_TXBUF txbuf;  //转发数据缓存
};

//server-busi
struct NETSRV_SRVBUSI
{
    //TRANSACTIONS    transQueue[NETSRV_TRANSACTION_QUEUE_NUMS];   //事务队列
};

//server-save
struct NETSRV_RECORDINFO
{
    int  rcdIndex;          //有内容的记录条数(无内容记录条数不存;同时也代表时间计数,rcdIndex每+1代表+30ms时间,rcdIndex/(1000/30) = 该记录秒数)
    int  rcdLength;         //记录长度(一次存储的内容长度)
    int  bufIndex;          //记录在服务器转发缓存中的tail索引
};

struct NETSRV_SAVEPROCDATA
{
    int  procCount;         //保存循环计数
    int  curInfoIndex;      //当前队列索引

    int  saveBufLen;        //单次保存buf长度
    unsigned char saveBuf[NETSRV_SRVSAVE_BUFLENTH]; //单次保存buf
    NETSRV_RECORDINFO  rcdInfoQueue[NETSRV_SRVSAVE_RCDINFONUM];
};

struct NETSRV_SRVSAVE
{
    int  useFlag;        //存储数据使用标志
    int  lockFlag;       //线程锁定(等待存储线程启用后解锁,目的是及时保存服务器创建角色的数据,供回放使用)

    char fileName[COMMON_STRLEN_512];

    NETSRV_SAVEPROCDATA procData;
};

//server-play
struct NETSRV_PLAYINFO
{
    int rcdIndex;           //记录条数(也代表时间计数概念)
    int rcdLength;          //记录长度
    int bufIndex;           //记录在回放缓存中的索引
};

struct NETSRV_PLAYPROCDATA
{
    int indexCount;     //回放流程循环次数(代表时间,indexCount+1代表时间+30ms.因记录索引rcdIndex也代表时间,故indexCount与rcdIndex进行比较作为时间控制)
    int headInfoIndex;  //send+1
    int tailInfoIndex;  //parse+1

    int fileBufIndex;    //buf当前解析索引
    unsigned char *fileBuf; //文件内容指针
    NETSRV_PLAYINFO playInfoQueue[NETSRV_SRVPLAY_INFONUM];
};

struct NETSRV_SRVPLAY
{
    int   useFlag;        //回放使用标志
    int   state;          //流程状态控制
    int   rate;           //回放倍速的值(1/2/3/4/5,目前所有的回放都是统一的)
    int   fileLen;        //文件大小
    int   mode;           //回放模式(1:一次性读取文件/2:缓存读取文件)
    int   bufLen;         //缓存大小

    char  fileName[COMMON_STRLEN_512];  //回放文件名

    NETSRV_PLAYPROCDATA procData;
};

struct NETSRV_SRVROOM
{
    int             useFlag;    //房间使用标志

    NETSRV_SRVCOMM  commData;   //服务器通讯数据
    NETSRV_SRVBUSI  busiData;   //业务
    NETSRV_SRVSAVE  saveData;   //存储
    NETSRV_SRVPLAY  playData;   //回放
};

struct NETSRV_SRVDATA
{
    int             uuid;               //服务器设备标识
    int             exeId;              //服务器程序标识

    int             listenSocket;       //监听的socket
    int             listenPort;         //监听端口

    int             serviceProcInter;   //服务线程间隔
    int             saveFileInter;      //存储间隔(< 服务器发送缓存全部更新时间,< NETSRV_SRVSAVE_RCDINFONUM*serviceProcInter)
    int             playRate;           //默认回放速率

    int             heartCheckInter;    //心跳检查间隔
    int             heartRecvTimeLimit; //心跳接收时限

    int             userThreadFlag;     //用户线程使用标志
    int             userThreadInter;    //用户线程轮询间隔(ms)

    int             forwardingMode;     //转发模式(网络服务器自行控制/用户控制)

    NETSRV_SRVROOM  roomData[COMM_SRVROOM_NUM];   //服务器房间数据
    NETSRV_RUNDATA  runData;
    NETSRV_SYSDATA  sysData;
};

//scommunicator-client
struct SCLIENT_COMM
{
    //para
    int     exeId;              //通道对应的程序ID

    int     connPort;           //客户端连接端口
    char    ipAddr[COMMON_STRLEN_32];   //客户端IP地址

    //data
    int     connFlag;         //通道连通标志
    int     socket;           //通道SOCKET
    int64_t connTime_ms;      //通道连接时刻(ms)
    int     lockFlag;         //连接过程锁,Server端不用(因connectChannel非原子操作,所以存在多线程同时connectChannel,的可能,故禁止同一时刻多个线程同时使用连接过程)

    int     heartSendWaitCount; //心跳发送循环次数
    int     heartRecvWaitCount; //心跳接收循环计数
    int64_t lastHeartCheckTime; //上次心跳检查时间(ms)

    //client rundata/sysdata
    COMMAND_DATA    cmdData;   //控制类数据
    COMM_CHANNEL_RXBUF rxbuf;  //接收buf
    COMM_CHANNEL_TXBUF txbuf;  //发送buf
};

struct NETSRV_CLIBUSI
{

};

struct NETSRV_CLIDATA
{
    int enableFlag;     //使用标志
    int uuid;           //计算机标识
    int roomId;         //房间号

    SCLIENT_COMM commData[COMM_CLICOMM_NUM];    //客户端通讯数据

    NETSRV_CLIBUSI busiData;   //业务数据
};

//communicator-client

struct CLIENT_RUNDATA
{
    int     recvAllFlow;  //接收总字节数
    int     recvAllFrame; //接收总帧数

    int     sendAllFlow;  //发送总字节数
    int     sendAllFrame; //发送总帧数
};

struct CLIENT_SYSDATA
{
    int     readProcLoopCount;      //读取线程轮循次数
    int     writeProcLoopCount;     //发送线程轮循次数
    int     dispatchProcLoopCount;  //dispatch线程轮循次数

    int     heartCheckCount;        //心跳检查计数
};

struct CLIENT_COMM
{
    //para
    int     heartCheckInter;    //心跳检查间隔
    int     heartSendTimeLimit; //心跳发送时限
    int     heartRecvTimeLimit; //心跳接收时限

    int     uuid;               //uuid
    int     exeId;              //程序标识

    int     port;       //服务器端口
    char    ipAddr[COMMON_STRLEN_32];   //服务器IP地址

    //data
    int     connFlag;         //通道连通标志
    int     socket;           //通道SOCKET
    int64_t connTime_ms;      //通道连接时刻(ms)
    int     lockFlag;         //连接过程锁(因connectChannel非原子操作,所以存在多线程同时connectChannel,的可能,故禁止同一时刻多个线程同时使用连接过程)

    int     heartSendWaitCount; //心跳发送循环次数
    int     heartRecvWaitCount; //心跳接收循环计数
    int64_t lastHeartCheckTime; //上次心跳检查时间(ms)

    CLIENT_RUNDATA     runData; //运行时数据
    CLIENT_SYSDATA     sysData; //系统数据

    COMMAND_DATA       cmdData; //控制类数据

    COMM_CHANNEL_RXBUF rxbuf;   //接收buf
    COMM_CHANNEL_TXBUF txbuf;   //发送buf
};

#endif //__MEM_DEF_H__
