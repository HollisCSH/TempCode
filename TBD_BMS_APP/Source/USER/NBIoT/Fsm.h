//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Fsm.h
//创建人  	: 
//创建日期	:
//描述	    : Sim模组FSM处理头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __FSM__H_
#define __FSM__H_

#ifdef __cplusplus
extern "C"{
#endif

//=============================================================================================
//包含头文件
//=============================================================================================
#include "BSPTypeDef.h"
#include "SimCommon.h"

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     MAX_GPS_COUNT   5
#define     MIN_GPS_COUNT   1

#define     NB_IOT_SEND_WIFI_PEROID     3600    //发送wifi辅助定位周期，秒
#define     NB_IOT_MAX_WAIT_TIME        300000  //最长执行NB模块上传wifi信号等待时间 ms

#define     FSM_TIMERID_POWERDOWN 	    0
#define     TIME_WAITFOR_END 			(2000)	//等待网络传输结束

#define     REFRESH_OVER_TIME           360000
//=============================================================================================
//定义数据类型
//=============================================================================================
//BMS状态
typedef enum _FSM_BMS_STATE
{
	 FSM_BAT_IDLE = 0	//空闲状态
	,FSM_BAT_CHG		//充电状态
	,FSM_BAT_DCHG	    //放电状态
    ,FSM_BAT_LOWSOC     //低电量状态
}FSM_BMS_STATE;

//FSM信息处理
typedef struct _tagMessage
{
    void* 	m_pHandler;
	
	uint32 	m_MsgID;
	uint32	m_Param1;
	uint32	m_Param2;
}Message;

//唤醒原因
typedef enum _WakeupType
{
	 WAKEUP_POWER = 0
	,WAKEUP_GYRO
	,WAKEUP_PMS
	,WAKEUP_SIM
	,WAKEUP_BLE
	,WAKEUP_RTC
	,WAKEUP_LOCK
#ifdef CFG_CABIN_LOCK	
	,WAKEUP_CABIN_LOCK
#endif	
	,WAKEUP_MAX
}WakeupType;

//=============================================================================================
//声明接口函数
//=============================================================================================

//FSM初始化
void Fsm_Init(void);

//FSM启动
void Fsm_Start(void);

//设置FSM活跃标志
void Fsm_SetActiveFlag(ActiveFlag af, Bool isActive);

//判断FSM是否空闲
Bool Fsm_CanGoSleep(void);

//设置电池的状态
void Fsm_BatterySetState(FSM_BMS_STATE state);

//获取FSM状态
FSM_BMS_STATE Fsm_Get(void);

//FSM放入信息id
void PostMsg(uint8 msgId);

//获取唤醒原因
WakeupType GetWakeUpType(void);

//设置唤醒原因
void SetWakeUpType(WakeupType type);

//FSM运行处理函数
void Fsm_Run(void);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of Fsm.h*****************************************/
