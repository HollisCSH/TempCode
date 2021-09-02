//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Gps.h
//创建人  	: 
//创建日期	:
//描述	    : SIM模组GPS定位头文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	:
//版本	    :
//修改日期	:
//描述	    : 
//=============================================================================================
#ifndef __GPS_H__
#define __GPS_H__

//=============================================================================================
//包含头文件
//=============================================================================================
#include "SimCommon.h"
#include "Sim.h"

#ifdef __cplusplus
extern "C"
{
#endif

//=============================================================================================
//宏参数设定
//=============================================================================================
#define     GET_GPS_TIME_SHORT	    2
#define     GET_GPS_TIME_LONG	    3

#define     GPS_ERR(_p2)            {SetErrorCode(ERR_TYPE_GPS, _p2, ET_ALARM);}

//=============================================================================================
//定义数据类型
//=============================================================================================
//GPS 模块状态机
typedef enum _GpsFsm
{
	 GPS_INIT = 0
	,GPS_START
}GpsFsm;

//GPS的经纬度
typedef struct _GpsCoor
{
	int32 Longitude;
	int32 Latitude;
}GpsCoor;

typedef struct _Gps
{	
	uint8  isLowPower:1;
	uint8  isLocation:1;
	uint8  isLocationUpdate:1;	//GPS定位是否被更新
	uint8  isPowerOn	 :1;
	uint8  isPowerOnNew	 :1;
    uint8  isStartGetInfo:1;    //GPS是否开始了定位查询
	uint8  reserved: 2;
	
	uint8  flagNorS; 		//0: North, 1:South
	uint8  flagEorW;		//0:East, 1:West
	uint8  state;
	uint8  timerSync; 		//本地是否需要同步
	
	uint16  hearbeatIntervalOld;
	uint16  hearbeatIntervalTemp;
	
	int32 longitude;	//经度
	int32 latitude;	    //纬度
	
	int32 altitude;	    //海拔
	float speed;		//速度,Km/h
	
	uint8 	sateInview; //可见卫星数 <GNSS Satellites in View>
	uint8  	maxSnr;

	SwTimer	m_Timer;
	uint8 	m_TimeOutS;
}Gps;

//=============================================================================================
//声明变量，供外部使用
//=============================================================================================
extern Gps * g_pGps;
extern Queue_old g_GpsQueue;           //GPS点队列

//=============================================================================================
//声明接口函数
//=============================================================================================

//GPS初始化
void Gps_Init(void);

//GPS处理信息打印函数
void Gps_Dump(void);

//是否需要获取定位信息
void Gps_Location(Bool isStart);

//GPS复位，gps结构体复位，下电
void Gps_Reset(void);

//GPS唤醒
void Gps_Wakeup(void);

//GPS开始启动
void Gps_Start(void);

//GPS进入睡眠
void Gps_Sleep(void);

//获取GPS经纬度，出队列，放在pbuf中
uint8 Gps_GetCoors(uint8* pBuf);

//获取GPS可见卫星数
uint8 Gps_GetSateInview(void);

//获取GPS强度
uint8 Gps_GetSateMaxSnr(void);

//计算GPS坐标
double Gps_CalcCoordinate(const char* strValue);

//确认坐标是否有效
Bool Gps_CoorIsValid(double longitude, double latitude);

//GPS延迟发送命令函数
void Gps_DelaySendCmdInd(int cmdInd, uint32 timeOutS);

//处理GPS发送命令
void Gps_TimerProc(void);

//GPS运行任务
void Gps_Run(void);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************end of Gps.c*****************************************/
