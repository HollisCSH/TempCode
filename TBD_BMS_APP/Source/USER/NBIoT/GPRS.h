//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Gprs.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组的2G/4G处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __GPRS_AT_H__
#define __GPRS_AT_H__

#ifdef __cplusplus
extern "C"
{
#endif 

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "sim.h"
#include "Utp.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define GPS_LOCATE_MODE     0x01    //协议指示GPS定位
#define LBS_LOCATE_MODE     0x02    //协议指示LBS定位
#define WIFI_LOCATE_MODE    0x03    //协议指示WIFI MAC定位

#define DISABLE_SLOW_CLK 	0
#define ENABLE_SLOW_CLK 	1

#define GPRS_EVENT_DEVICE_RESET 	0x01	//设备复位
#define GPRS_EVENT_PORT_CHANGED 	0x02	//槽位状态改变
#define GPRS_EVENT_DEV_CHANGED 		0x04	//设备状态,点火状态改变
	
#define ATCRC_SIMCARD_INVALID  	ATCRC_DONE
#define ATCRC_NETWORK_REG_FAIL 	ATCRC_DONE
#define ATCRC_REG_DENIED 		ATCRC_DONE
	
#define TCP_UNKNOWN         0x00
#define TCP_CONNECT_OK   	0x01
#define TCP_CLOSE  			0x02
#define IP_INITIAL          0x03
#define PDP_DECT			0x04
#define TCP_ERROR           0xff

#define AT_OK "\r\nOK\r\n"

#define ERR(_p2) {SetErrorCode(ERR_TYPE_GPRS, _p2, ET_ALARM);}

/*
Unsolicited TCP/IP command <err> Codes
0 operation succeeded
1 Network failure
2 Network not opened
3 Wrong parameter
4 Operation not supported
5 Failed to create socket
6 Failed to bind socket
7 TCP server is already listening
8 Busy
9 Sockets opened
10 Timeout
11 DNS parse failed for AT+CIPOPEN
255 Unknown error
*/

#define GPRS_PROTOCOL_VER  0x0A	

#define MAX_POST_LOG_COUNT 20

#define TIME_HEARBEAT_SHORT	100

#define TIME_HEARBEAT_LONG	60000
#define TIME_HEARBEAT_LONG_LONG	60000

#define GPRS_SENDDATA_TIME_SHORT  3000
#define GPRS_SENDDATA_TIME_LONG   30000

#define SPLIT_LINE	//NOT USED

//=============================================================================================
//定义数据类型
//=============================================================================================
enum
{
	TIMERID_HEARBEAT
};

//GPRS 相关的命令
typedef enum _GPRS_CMD
{
	//Request
	 GPRS_RESET    			= 0x01  
	,GPRS_PORT_STATE_CHANGED=0x02
	,GPRS_TIME_SYNC			=0x04
	,GPRS_GET_ORDER_STATE	=0x05
#ifdef GPRS_FW_UPGRADE
	,GPRS_FW_STATE_CHANGED	=0x07
#endif
#ifdef CFG_LOG
	,GPRS_POST_LOG			=0x08
#endif
	,GPRS_EVENT_CHANGED     =0x09
	,GPRS_REQ_MAX			=0x1F

	 //Notify
	,GPRS_HEART_BEAT = 0x20     //心跳
	,GPRS_GPS_POINT  = 0x21
	,GPRS_WAKEINTR_CHANGE  = 0x22   //定时唤醒间隔改变
	,WIFI_MAC_POINT  = 0x23   //WIFI MAC地址点    

	 //Command
	,GPRS_DEV_RESET	 		 = 0x40
	,GPRS_FORBID_DISCHARGE	 = 0x51
	,GPRS_ACTIVE_DEVICE		 = 0x52
	,GPRS_SET_DEV_STATE		 = 0x53
#ifdef CFG_LOG
	,GPRS_LOG_GET_INFO 		 = 0x54
	,GPRS_BAT_VERIFY 	     = 0x55
	,GPRS_LOG_READ	 		 = 0x56
#else
	,GPRS_BAT_VERIFY 	     = 0x55
#endif
#ifdef CFG_WHEEL_LOCK
	,GPRS_SET_LOCK 	         = 0x57
#endif
#ifdef CFG_CABIN_LOCK
	,GPRS_SET_CABIN_LOCK 	 = 0x58
#endif
	,GPRS_SET_ACC		 	 = 0x59
#ifdef CFG_BEACON
	,GPRS_SET_BEACON 		= 0x5A
#endif
	
	,GPRS_CMD_MAX
}GPRS_CMD;

//GPRS的相关状态
enum
{
	 GPRS_INIT    = 0
	,GPRS_START
	,GPRS_CALL_READY	
	,GPRS_READY
	,GPRS_RUN
};

//SIM卡的相关状态
enum
{
	 SIMCARD_UNKNOWN = 0
	,SIMCARD_READY = 1
	,SIMCARD_NOT_INSERTED = 2
	,SIMCARD_NOT_READY 	  = 3
};

//GPRS连接状态
enum
{
	 GPRS_LINK_BROKEN_UNKNOWN = 0
	,GPRS_LINK_BROKEN_CLOSED
	,GPRS_LINK_BROKEN_IPINIT
	,GPRS_LINK_BROKEN_PDP_DEACT
};


typedef OP_CODE (*GprsReqProcFn)(uint8 req, const uint8* data, int len, uint8* pRsp, uint8* rspLen);
typedef Bool (*GprsRspProcFn)(uint8 req, OP_CODE op, const uint8* data, int len);
typedef int (*GetPortStatePktFn)(uint8* pBuff, int len);
typedef Bool (*IsSendPortChangedFn)(void);

//GPRS 总处理结构体
typedef struct _Gprs
{
	uint8 isAuth  		:1;
	uint8 isIndentify	:1;
	uint8 isLowPower	:1;     //是否电源关闭，1代表关闭
	uint8 isConnected	:1;     //是否已连接，1代表连接
	uint8 isGatt		:1;     //是否附着网络，1代表附着
	uint8 isReg			:1;
	uint8 isGetLock		:1;
	uint8 isRcvSms		:1;
	
	uint8 isSimReady	:2;
	uint8 isSyncTime	:1;
	uint8 isSendSyncTime:1;
	uint8 isReadSms		:1;
	uint8 isForceSendLock	:1;
	uint8 serverLock	:1;
	uint8 reportedLock	:1;
	
	uint8 isSendSms		:1;
	uint8 isSendGps		:1;
	uint8 isSendLbs		:1;    
	uint8 isSendWakeIntr:1;       
	uint8 isSendWifiMac :1;     
	uint8 reserved		:3;
	
#ifdef CFG_BEACON
	uint8 isSendBeacon:1;
	uint8 isSendCabinLock:1;
	uint8 reserved01:6;
#endif
	
	uint8 g_ReportedPort;
	uint8 g_ServerPort;
	uint8 resetCounter;
	uint8 SendRstCnt;   //发送复位命令登陆次数
	
	uint8 timeSync;

	uint16 		tcpPktlen;
	const uint8* pTcpPkt;   
    const uint8* pTcpTranPkt;//经过转换后的数据
    
	SwTimer m_HbTimer;

	uint32 heartBeatInterval;
	uint8  m_TxInitPacket;
	uint8  m_HbCount;
	uint8  m_GpsCount;
	uint8  m_WifiCount;

	time_t m_syncSeconds;
#ifdef SIM868
	Bool m_isStart;
#endif

	GprsReqProcFn reqProc;
	GprsRspProcFn rspProc;
	void (*Run)();
	int (*GetDataPkt)(const AtCmdItem* pCmd, uint8* buff, int bufLen);
	GetPortStatePktFn GetPortStatePkt; 
	IsSendPortChangedFn	  IsSendPortChanged;
}Gprs;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern Gprs * g_pGprs;
extern SwTimer g_GprsCmdTimer;

//=============================================================================================
//声明接口函数
//=============================================================================================

//GPRS基本初始化
void Gprs_InitBase(void);

//GPRS 2G通信初始化函数
void Gprs_Init(void);

//GPRS 2G复位变量和停止定时器函数
void Gprs_Reset(void);

//GPRS启动
void Gprs_Start(void);

//GPRS唤醒
void Gprs_Wakeup(void);

//GPRS睡眠
void Gprs_Sleep(void);

//Gprs watch dog opertion
//启动gprs看门狗定时器，300S后启动，5Min的喂狗阈值
void Gprs_WdStart(void);

//看门狗停止
void Gprs_WdStop(void);

//喂狗，重新启动定时器
void Gprs_WdFeed(void);

//看门狗检测，超时喂狗则重新上电SIM模块
void Gprs_WdCheck(void);

//GPRS处理信息打印
void Gprs_Dump(void);

#ifdef CFG_SIM_SLEEP
//GPRS设置睡眠
void Gprs_SetCsclk(uint8 csclk);

//GPRS是否设置睡眠
Bool Gprs_IsSlowClk(void);

//GPRS获取睡眠状态
uint8 Gprs_GetCsclk(void) ;
#endif

//GPRS延迟发送函数
void Gprs_DelaySendCmdInd(int cmdInd, uint32 timeOutS);

//获取更新标志，暂不使用
void Gprs_UpdateFlag(uint8 flag);

//GPRS数据打包函数
int Gprs_FramePkt(uint8 cmd, const void* pData, int dataLen, void* pBuf, int bufLen);

//GPRS打包DeviceReset Request复位请求
uint8 Gprs_GetDeviceResetPkt(void* pBuf, int bufLen);

//GPRS打包定时唤醒间隔请求
uint8 Gprs_GetWakeUpInternalPkt(void* pBuf, int bufLen);

//回复服务器发送过来的请求
void Gprs_TCP_SendData(const uint8* data, uint16 len);

//TCP数据发送
Bool Gprs_SendData(const AtCmdItem* pCmd);

//验证payload数据包头部尾部
Bool Gprs_VerifyIpPkt(const uint8* pData, int datalen);

//验证tcp数据包
Bool Gprs_VerifyIpDataPkt(const char* pReq, int reqLen, const uint8** ppData, int* datalen);

//payload数据包处理过程
void Gprs_UtpRspProc(uint8 req, OP_CODE op, const uint8* data, int len);

//判断GPRS是否允许重发
Bool Gprs_IsAllowResend(AtCmdCtrl* pAtCmdCtrl, const AtCmdItem* pAtCmd, int reTxCount);

//检查req是否我一个完整的req数据包,针对命令+IPDATA,<len>,<data>,\r\n特殊处理，因为该数据包的数据部分有可能包含\r\n.
Bool Gprs_Filter(const char* pRcv, uint16* len);

//GPRS命令处理过程
void Gprs_CmdProc(const UtpFrame* pFrame);

//改变心跳时间
void Gprs_SetHearbeatInterval(uint32 ms);

//GPRS开始等待响应
void Gprs_WaitForRsp(uint32 ms);

//GPRS是否在等待响应
Bool Gprs_IsWaitRsp(void);

//GPRS发送端口变化，此处不使用
Bool Gprs_IsPortChanged(void);

//打包电池槽发生变化的函数，暂不使用
int Gprs_GetPortStateChangedPkt(void* pBuf, int bufLen);

//GPRS电池换电变化，此处不使用
int Bunker_GetPortStateChangedPkt(uint8* pBuf, int bufLen);

//GPRS数据发送打包函数：打包GPS经纬度数据和心跳数据
int Gprs_GetDataPkt(const AtCmdItem* pCmd, uint8* buff, int bufLen);

//Gprs获取心跳包数据组装
uint8 Gprs_GetHeartbeatPkt(void* pBuf, int len);

//GPRS定时器处理任务
Bool Gprs_TimerProc(void);

//GPRS运行任务
void Gprs_Run(void);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of Gprs.h*****************************************/
