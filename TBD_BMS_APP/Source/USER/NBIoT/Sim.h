//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Sim.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组SIM处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __SIM_H__
#define __SIM_H__

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "AtCmdCtrl.h"
#include "Gprs.h"
#include "Gps.h"

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     AT_OK_STR 		        "\r\nOK\r\n"
#define     AT_ERROR_STR 	        "\r\nERROR\r\n"

#define     SIM_TIME_ID_POWERON1   0
#define     SIM_TIME_ID_POWERON2   1
#define     SIM_TIME_ID_RESET      2
#define     SIM_TIME_ID_READY      3

#define     POWERE_OFF_DEFAULT     3000

//=============================================================================================
//定义数据类型
//=============================================================================================
//Sim模组状态
typedef enum _SimState
{
	 SIM_POWEROFF = 0
	,SIM_POWERON
	,SIM_READY
	,SIM_SMS
}SimState;

//Sim模组复位原因
typedef enum _SimResetReason
{
	 SIM_START = 0
	,SIM_GPRS_WDT
	,SIM_ATCMD_ERR		//AT命令错误重试失败
	,SIM_GPRS_DEV_RESET	//网络远程复位
	,SIM_CIPOPEN_ERROR
	,SIM_SHUTDOWN_ERROR
	,SIM_CIPSTATUS_ERROR
    ,SIM_NO_CONNECT
	,SIM_CPIN_NOT_READY
	,SIM_CPIN_NOT_INSERTED
	,SIM_CONNECT_ERROR  //连接服务器错误，找不到服务器
	,SIM_SHELL
	,SIM_READY_TIMEOUT
	
	,SIM_UPG_ERROR
	,SIM_UPG_SWITCH
}SimResetReason;

typedef Bool (*SimSendCmdItemFn)(const AtCmdItem* pCmd, char* buf, uint32* delayMs);
//SIM卡的状态
typedef struct _SimCard
{
	uint8  	isSimSleep:1;       //睡眠时，此状态为1
	uint8  	isSimSleepNew:1;    //唤醒时，此状态为1
    uint8   isWakeFromSleep:1;  //从睡眠中唤醒标志
    uint8   isSimStartOK;       //是否唤醒成功
	uint8  	reserved:4;
	
	SimState 	state;          //SIM的状态
	
	uint8  	isCommOk;
	uint8  	isReset;
	uint8  	isLowPower;
    uint8   isShutDown;
    
	char 	iccid[CCID_LEN+1];
	uint16 	CC;  //Country Code
	uint8  	csq;	

	SimSendCmdItemFn SendCmdItem;
}SimCard;

//Sim模组唤醒状态机
typedef enum _SimWake
{
    Sim_Wake_Init = 0,
    Sim_Wake_OK,
    Sim_Wake_Fail,
}
SimWakeFsm;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern SimCard* g_pSimCard;
extern AtCmdCtrl* g_pSimAtCmdCtrl;
extern u32 gSimStartTim;
extern uint32 g_SimPowerOffDelay; //上电延时变量

//=============================================================================================
//声明接口函数
//=============================================================================================

//SIM初始化，进行g_pSimAtCmdCtrl这个结构体的注册
void Sim_Init(void);

//SIM复位函数
void Sim_PowerReset(uint8 reason);

//重新执行上电流程
void Sim_Start(void);

//SIM模组是否可以进入睡眠
Bool Sim_CanGoSleep(void);

//SIM模组是否睡眠
Bool Sim_IsSleep(void);

//SIM进入睡眠
Bool Sim_Sleep(void);

//SIM模组是否关机
Bool Sim_IsShutDown(void);

//Sim模组唤醒
//回复false为唤醒失败，true为唤醒成功
Bool Sim_Wakeup(void);

//SIM模块上电
void Sim_DoPowerOn(void);

//SIM模块下电
void Sim_PowerOff(void);

//SIM模块关机
void Sim_ShutDown(void);

//SIM接收放在UART中断接收函数进行接收处理
void Sim_RxByte(uint8 data);

//SIM发送AT命令函数
Bool Sim_TxAtCmd(const char* pData, int len);

//SIM模组定时器启动
void Sim_TimerStart(uint32 timeOutMs, uint32 nId);

//判断是否接收结束
Bool Sim_Filter(const char* pRcv, uint16* len);

//设置发送延时
void Sim_SetTxDataTime(uint32 ms);

//暂不使用
int Sim_SyncSendAtCmdEx(const char* pAtCmd, const char* pAck, int waitMs, char** pRsp);

//同步发送pAtCmd命令字符串，等待接收响应
int Sim_SyncSendAtCmd(const char* pAtCmd, const char* pAck, int waitMs);

//异步发送命令
int Sim_AnsySendAtCmd(const char* pAtCmd, const char* pAck, int waitMs, uint32 nTag);

//延迟timeOutMs时间后根据命令索引发送数据
void Sim_DelaySendCmdInd(int cmdInd, uint32 timeOutMs);

//根据命令索引发送数据
Bool Sim_SendCmdInd(int cmdInd);

//SIM模组获取命令索引
AtCmdItem* Sim_GetCmdByInd(int cmdInd);

//SIM模组根据命令发送数据
Bool Sim_SendCmdItem(const AtCmdItem* pCmd);

//SIM模块上电处理过程代码
void Sim_TimerProc(void);

//SIM模组运行任务
void Sim_Run(void);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of Sim.h*****************************************/
