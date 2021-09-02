//=======================================Copyright(c)===========================================
// 								  深圳易马达科技有限公司
//---------------------------------------文件信息----------------------------------------------
//文件名   	: Gps.c
//创建人  	: 
//创建日期	:
//描述	    : SIM模组GPS定位文件
//-----------------------------------------------当前版本修订----------------------------------
//修改人   	: Handry
//版本	    : GPRS协议1.3
//修改日期	: GPS点增加SOC
//描述	    : 
//=============================================================================================

//=============================================================================================
//包含头文件
//=============================================================================================
#include "Gps.h"
#include "BmsSystem.h"
#include "DateTime.h"
#include "AtCmdTab.h"
#include "sim.h"

//=============================================================================================
//定义全局变量
//=============================================================================================
static Gps __g_Gps;
Gps * g_pGps = &__g_Gps;

Queue_old g_GpsQueue;           //GPS点队列
static GpsCoor g_GpsCoors[5];

//判断坐标是否有效算法:判断相邻2个点的距离是否超过指定值
static double g_compareLongitude = 0.0;

//=============================================================================================
//定义接口函数
//=============================================================================================

//GPS初始化
void Gps_Init()
{
	memset(g_pGps, 0, sizeof(Gps));
	g_pGps->m_TimeOutS = GET_GPS_TIME_LONG;
	
	g_pGps->longitude = 0.0;
	g_pGps->latitude  = 0.0;

    //初始化队列，队列含有10个元素
	QUEUE_init(&g_GpsQueue, &g_GpsCoors, sizeof(GpsCoor), GET_ELEMENT_COUNT(g_GpsCoors));
}

//GPS处理信息打印函数
void Gps_Dump()
{
//	Printf("Gps dump:\n");
//	Printf("\t SateInView=%d, MaxSnr=%d.\r\n", g_pGps->sateInview, g_pGps->maxSnr);
//	Printf("\t isLowPower = %d\n"	, g_pGps->isLowPower);
// 	Printf("\t state = %d\n"		, g_pGps->state);
// 	Printf("\t isLocationUpdate = %d\n"		, g_pGps->isLocationUpdate);
// 	Printf("\t Time.isStart = %d\n", g_pGps->m_Timer.m_isStart);
// 	Printf("\t Time.timeOutTicks = %d\n", g_pGps->m_Timer.m_TimeOutTicks);
// 	Printf("\t GpsQueue Count = %d\n", QUEUE_getElementCount(&g_GpsQueue));
}

//是否需要获取定位信息
void Gps_Location(Bool isStart)
{
	if(g_pGps->isLocation == isStart) return;

	g_pGps->isLocation = isStart;
	
	if(isStart)
	{
		g_pGps->m_TimeOutS = GET_GPS_TIME_LONG;
		Gps_DelaySendCmdInd(GPS_CMD_CGPSINFOCFG, 1);
	}
	else
	{
		SwTimer_Stop(&g_pGps->m_Timer);
	}
}

//GPS复位，gps结构体复位，下电
void Gps_Reset()
{
	GPS_ERR(0);
	SwTimer_Stop(&g_pGps->m_Timer);
	g_pGps->state = GPS_INIT;
	g_pGps->timerSync = False;
	g_pGps->isLocation = False;
	g_pGps->isPowerOn = False;
	
	g_pGps->m_TimeOutS = GET_GPS_TIME_LONG;
	g_pGps->sateInview = 0;
	g_pGps->maxSnr	   = 0;
	g_pGps->isLocationUpdate = False;
	SIM_ANT_PWR_OFF();
	g_compareLongitude = 0.0;
    QUEUE_removeAll(&g_GpsQueue);
}

//GPS唤醒
void Gps_Wakeup()
{
	g_pGps->isPowerOnNew = True;
	g_pGps->isPowerOn = False;
	g_pGps->m_TimeOutS = GET_GPS_TIME_LONG;
	SIM_ANT_PWR_ON();           //GPS天线上电
	SwTimer_Stop(&g_pGps->m_Timer);
}

//GPS开始启动
void Gps_Start()
{
	Gps_Wakeup();
	g_pGps->state = GPS_START; 
}

//GPS进入睡眠
void Gps_Sleep()
{
	//当动力电池在时，GPS模组电源不关闭
//	if(!IS_BAT_ON())
//	{
//		g_pGps->isPowerOn = True;
//		g_pGps->isPowerOnNew = False;
//		SIM_ANT_PWR_OFF();
//	}
    
    g_pGps->isPowerOn = True;
    g_pGps->isPowerOnNew = False;
    g_pGps->isStartGetInfo = False;
    SIM_ANT_PWR_OFF();

    g_pGps->isLocation = False;
	g_pGps->isLocationUpdate = False;
	SwTimer_Stop(&g_pGps->m_Timer);
}

//获取GPS经纬度，出队列，放在pbuf中
uint8 Gps_GetCoors(uint8* pBuf)
{
    // 修改，增加定位模式指示
	int i = 3;
	GpsCoor* pGpsCoor = Null;
    
    //增加SOC显示
    pBuf[0] = gBatteryInfo.Data.FiltSOC & 0xff;
    pBuf[1] = (gBatteryInfo.Data.FiltSOC >> 8) & 0xff;

    //GPS定位
	if(g_pGprs->isSendGps)
	{
		pBuf[2] = GPS_LOCATE_MODE;
	}
    //基站定位
    else if(g_pGprs->isSendLbs)
    {
		pBuf[2] = LBS_LOCATE_MODE;
    }
    /*
	int i = 0;
	GpsCoor* pGpsCoor = Null;    
    */
	for(pGpsCoor = (GpsCoor*)QUEUE_getHead(&g_GpsQueue); pGpsCoor; QUEUE_removeHead(&g_GpsQueue), pGpsCoor = (GpsCoor*)QUEUE_getHead(&g_GpsQueue))
	{
		memcpy(&pBuf[i], pGpsCoor, sizeof(GpsCoor));
		i+= sizeof(GpsCoor);
	}

	return i;
}

//获取GPS可见卫星数
uint8 Gps_GetSateInview()
{
	return g_pGps->sateInview;
}

//获取GPS强度
uint8 Gps_GetSateMaxSnr()
{
	return g_pGps->maxSnr;
}

//计算GPS坐标
double Gps_CalcCoordinate(const char* strValue)
{
	int32 deg;
	double fValue = 0.0;
	
	if(sscanf(strValue, "%lf", &fValue) == 0) return 0.0;
	
	fValue = fValue/100.0;
	deg = (int32)fValue;
	fValue -= deg;
	
	return (deg + fValue / 0.6);
}

//确认坐标是否有效
Bool Gps_CoorIsValid(double longitude, double latitude)
{
	#define MAX_DISTANCE_PRE_SECOND (25000.0/60.0/60.0 + 50/*GPS误差范围*/)	//每秒的最长距离,米
	Bool bRet = False;
	
	static S_RTC_TIME_DATA_T g_compareDt = {0};
	static double g_comparelatitude = 0.0;
	
	double distance = 0.0;
	uint32 seconds = 0;
	S_RTC_TIME_DATA_T newDt;

	if(g_compareDt.u32Year == 0 || g_compareLongitude == 0.0)
	{
		goto End;
	}
	else
	{
		RTC_GetDateAndTime(&newDt);
	}

	seconds = DateTime_Sub(Null, &g_compareDt);

	if(0 == seconds || seconds >= 60)
	{
		goto End;
	}
	
	distance = get_distance(latitude, longitude, g_comparelatitude, g_compareLongitude) * 1000;


	//如果距离在合理的速度移动范围之内，认为是有效点。
	if(distance < MAX_DISTANCE_PRE_SECOND * seconds)
	{
		bRet = True;
	}
	
//#ifdef GPS_TEST
	//Only for test
	if(!bRet)
	{
		//static uint32 counter = 0;
		//Printf("%08d: %.08f,%.08f, %.01f, %.01f\n", counter++, latitude, longitude, distance, (MAX_DISTANCE_PRE_SECOND * seconds));
	}
//#endif

End:
	g_compareLongitude = longitude;
	g_comparelatitude  = latitude;
	RTC_GetDateAndTime(&g_compareDt);
	return bRet;
	
}

//GPS延迟发送命令函数
void Gps_DelaySendCmdInd(int cmdInd, uint32 timeOutS)
{
	AtCmdItem* pCmd = Sim_GetCmdByInd(cmdInd);
	SwTimer_Start(&g_pGps->m_Timer, timeOutS * 1000, (uint32)pCmd);
}

//处理GPS发送命令
void Gps_TimerProc()
{
	if(SwTimer_isTimerOut(&g_pGps->m_Timer))
	{
		Sim_SendCmdItem((AtCmdItem*)g_pGps->m_Timer.m_Id);
	}
}

//GPS运行任务
void Gps_Run()
{
    //未启动，则返回
	if(g_pGps->state != GPS_START && g_pGps->isLowPower) 
	{
		return;
	}

    //未上电，则返回
	if(!g_pGps->isPowerOn)
	{
		return;
	}
	
	if(!AtCmdCtrl_IsIdle(g_pSimAtCmdCtrl)) 
		return;

	Gps_TimerProc();
	
    //周期启动获取GPS点信息定时器。5S周期
    #ifdef GSM_USE_EPO   //是否使用EPO秒定位功能
    
    #else
    
        #ifdef  OPEN_GPS_BEFORE_GSM
        ;
        #else
        if(!SwTimer_IsStart(&g_pGps->m_Timer))
        {
            SwTimer_Start(&g_pGps->m_Timer, g_pGps->m_TimeOutS*1000, (uint32)Sim_GetCmdByInd(GPS_CMD_CGPSINFOCFG));
        }
        #endif
    #endif
}

/*****************************************end of Gps.c*****************************************/
