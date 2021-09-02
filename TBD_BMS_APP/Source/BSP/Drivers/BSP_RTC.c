/****************************************************************************/
/* 	File    	BSP_RTC.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 				HAl库读取RTC时 先获取时间再获取日期 顺序错误会导致读锁定
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_RTC.h"
#include "BSP_UART.h"
#include "stdio.h"

/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
static u8 gRTCTimeUp = 0;  //唤醒变量
RTC_HandleTypeDef hrtc;
/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/

/*****************************************************
	* 函数功能: RTC 时钟、中断初始化
	* 输入参数: RTC_HandleTypeDef 类型
	* 返 回 值: 无
	* 说    明：该类函数在调用HAL_XX_Init 会内联
******************************************************/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
  }
  HAL_NVIC_SetPriority(RTC_TAMP_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(RTC_TAMP_IRQn);		
}

/*****************************************************
	* 函数功能: RTC 初始化
	* 输入参数: RTC_HandleTypeDef rtc类型
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void MX_RTC_Init(void)
{        
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
	/*这里并不是每次上电都要初始化RTC，根据第0个备份寄存器的数组决定。因为在不断复位启动执行吃时候RTC，
	RTC的时间将会变慢，大概一次复位将会慢半秒到一秒，这个真的可怕啊！我的项目应用是需要在stop模式下唤
	醒并复位，相当于不断时间变成一半，这个问题搞了我快两天的工作时间，真是HAL的坑啊！20201204*/
	hrtc.TampOffset = (TAMP_BASE - RTC_BASE);//一定加这句，不然在不初始化HAL_RTC_Init()的时候，执行HAL_RTCEx_BKUPRead(&RtcHandle,RTC_BKP_DR0)读取不成功且为0
	hrtc.Instance    = RTC;    
	if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1)!=0X5050)		//是否第一次配置
	{    
        
        hrtc.Init.HourFormat 	= RTC_HOURFORMAT_24;			//RTC设置为24小时格式 
        hrtc.Init.AsynchPrediv 	= 127;							//异步分频系数 默认值
        hrtc.Init.SynchPrediv 	= 255;							//同步分频系数 默认值
        hrtc.Init.OutPut 		= RTC_OUTPUT_DISABLE;	//以下为闹钟配置
        hrtc.Init.OutPutRemap 	= RTC_OUTPUT_REMAP_NONE;
        hrtc.Init.OutPutPolarity= RTC_OUTPUT_POLARITY_HIGH;
        hrtc.Init.OutPutType 	= RTC_OUTPUT_TYPE_OPENDRAIN;
        hrtc.Init.OutPutPullUp 	= RTC_OUTPUT_PULLUP_NONE;  
        HAL_RTC_Init(&hrtc);
		RTC_SetInfo(&DefaultInfo);							//配置默认值
        
		HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0X5050);		//标记已经初始化过了
        HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1);
	}
}

/*****************************************************
	* 函数功能: RTC 初始化
	* 输入参数: RTC_HandleTypeDef rtc类型
	* 返 回 值: 无
	* 说    明：适配外部代码
******************************************************/
void BSPRTCInit(void)
{
	MX_RTC_Init();
}

/*****************************************************
	* 函数功能: 获取当前芯片时间
	* 输入参数: RTC_InfoTypeDef *NewInfo
	* 返 回 值: 返回0：获取成功  1：时间获取错误 2：日期获取错误
	* 说    明：无
******************************************************/
uint8_t RTC_GetInfo(t_PCF85063_DATE *NewInfo)
{
//	RTC_InfoTypeDef *Info;
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	if (HAL_RTC_GetTime(&hrtc,&sTime,RTC_FORMAT_BCD) != HAL_OK)
	{
		_UN_NB_Printf("sTime error");
		return 1;	
	}

	if (HAL_RTC_GetDate(&hrtc,&sDate,RTC_FORMAT_BCD) != HAL_OK)
	{
		_UN_NB_Printf("sDate error");
		return 2;	
	}		
	NewInfo->year 	= sDate.Year;
	NewInfo->month 	= sDate.Month;
	NewInfo->day 	= sDate.Date;
	NewInfo->hour 	= sTime.Hours;
	NewInfo->minute = sTime.Minutes;
	NewInfo->second = sTime.Seconds;
	return 0;		
}

/*****************************************************
	* 函数功能: 设置当前芯片时间
	* 输入参数: RTC_InfoTypeDef *NewInfo
	* 返 回 值: 返回0：设置成功  1：时间设置错误 2：日期设置错误
	* 说    明：星期几不使用，但不可以不配置值
******************************************************/
uint8_t RTC_SetInfo(t_PCF85063_DATE *SetInfo)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	volatile t_PCF85063_DATE NewInfo = *SetInfo;
	//	sTime.Hours 	= NewInfo->pTime.Hours;	
	//  sTime.Minutes =	NewInfo->pTime.Minutes;
	//  sTime.Seconds = NewInfo->pTime.Seconds;
	sTime.Hours 	= NewInfo.hour;	
	sTime.Minutes   = NewInfo.minute;
	sTime.Seconds   = NewInfo.second;
	sTime.SubSeconds 			= 0;
	sTime.DayLightSaving 	= RTC_DAYLIGHTSAVING_NONE;	
	sTime.StoreOperation 	= RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
		return 1;

	//  sDate.WeekDay = NewInfo->pData.WeekDay;
	//  sDate.Month 	= NewInfo->pData.Month;;
	//  sDate.Date 		= NewInfo->pData.Date;
	//  sDate.Year 		= NewInfo->pData.Year;
	sDate.Date 		= NewInfo.day;
	sDate.WeekDay   = RTC_WEEKDAY_TUESDAY;    
	sDate.Month 	= NewInfo.month;
	sDate.Year 		= NewInfo.year;
	if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
		return 2;
	return 0;
}

//=============================================================================================
//函数名称	: void BSPRTCSetAlarmAfterSeconds(u32 sec)
//输入参数	: sec:秒	
//输出参数	: void
//静态变量	: 
//功    能	: 设置RTC在sec秒后产生RTC中断
//注    意	: 可以设置两个闹钟A/B,此处定时最长为65536秒 BCD码做加法计算需转换
//=============================================================================================
void BSPRTCSetAlarmAfterSeconds(uint16_t sec)
{
	t_PCF85063_DATE pRdTimeAndDate;
	uint8_t pHours,pMinutes,pSeconds;
	RTC_AlarmTypeDef sAlarm = {0};
	RTC_GetInfo(&pRdTimeAndDate);
	pHours    = sec/3600;         //获取增加的时分秒
	pMinutes  = sec%3600/60;
	pSeconds  = sec%60;
	if((RTC_Bcd2ToByte(pRdTimeAndDate.second)+pSeconds) >= 60)
	{
		pMinutes++;
		sAlarm.AlarmTime.Seconds = RTC_ByteToBcd2((RTC_Bcd2ToByte(pRdTimeAndDate.second)+pSeconds)-60);
	}  
	else 
		sAlarm.AlarmTime.Seconds = RTC_ByteToBcd2((RTC_Bcd2ToByte(pRdTimeAndDate.second)+pSeconds));
	if((RTC_Bcd2ToByte(pRdTimeAndDate.minute)+pMinutes) >= 60)
	{
		pHours++;    
		sAlarm.AlarmTime.Minutes = RTC_ByteToBcd2((RTC_Bcd2ToByte(pRdTimeAndDate.minute)+pMinutes)-60);
	}
	else 
		sAlarm.AlarmTime.Minutes = RTC_ByteToBcd2((RTC_Bcd2ToByte(pRdTimeAndDate.minute)+pMinutes));
	if((RTC_Bcd2ToByte(pRdTimeAndDate.hour)+pHours) >= 24)
	{
		sAlarm.AlarmTime.Hours = RTC_ByteToBcd2((RTC_Bcd2ToByte(pRdTimeAndDate.hour)+pHours)-24);
	}
	else 
		sAlarm.AlarmTime.Hours = RTC_ByteToBcd2((RTC_Bcd2ToByte(pRdTimeAndDate.hour)+pHours));

	sAlarm.Alarm = RTC_ALARM_A;                                     //A 事件对应A回调函数
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;   //闹钟日期或者星期的选择
	sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;                   //具体的日期或者星期几
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY ;                  //时分秒匹配即产生中断
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;        //闹钟秒设置掩码
	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);            //开启闹钟中断
}

//=============================================================================================
//函数名称	: void BSPRTCStop(void)
//输入参数	: void
//输出参数	: void
//静态变量	:
//功    能	: RTC停止函数 
//注    意	:
//=============================================================================================
void BSPRTCStop(void)
{
    HAL_RTC_DeactivateAlarm(&hrtc , RTC_ALARM_A);
}

//=============================================================================================
//函数名称	: void BSPRTCGetTimeUp(void)
//输入参数	: void
//输出参数	: 1：计时已满；0：计时未满
//静态变量	:
//功    能	: 获取RTC计时标志
//注    意	:
//=============================================================================================
u8 BSPRTCGetTimeUp(void)
{
    return gRTCTimeUp;
}

//=============================================================================================
//函数名称	: void BSPRTCClrTimeUp(void)
//输入参数	: void
//输出参数	: void
//静态变量	:
//功    能	: 清除RTC计时标志
//注    意	:
//=============================================================================================
void BSPRTCClrTimeUp(void)
{
    gRTCTimeUp = 0;
}
 
//=============================================================================================
//函数名称	: void HAL_RTC_AlarmAEventCallback
//输入参数	: void
//输出参数	: void
//静态变量	:
//功    能	: RTC Alarm中断处理函数
//注    意	:
//=============================================================================================
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	/*用作MCU的周期唤醒 */
	gRTCTimeUp = 1;
	_UN_NB_Printf("RTC Alarm A\n");
}




/*****************************************************
	* 函数功能: 读取时间测试
	* 输入参数: void
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void RTC_ReadTest(void)
{
	RTC_InfoTypeDef NewInfo;
	HAL_RTC_GetTime(&hrtc,&(NewInfo.pTime),0);
	HAL_RTC_GetDate(&hrtc,&(NewInfo.pData),0);
	_UN_NB_Printf("Now:Y-%d M-%d D-%d W-%d H-%d M-%d S-%d\n",NewInfo.pData.Year,NewInfo.pData.Month,NewInfo.pData.Date,
	NewInfo.pData.WeekDay,NewInfo.pTime.Hours,NewInfo.pTime.Minutes,NewInfo.pTime.Seconds);
	_UN_NB_Printf("Now:Ss-%d",NewInfo.pTime.SubSeconds);
}


