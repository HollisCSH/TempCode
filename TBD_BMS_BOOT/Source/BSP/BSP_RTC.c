/****************************************************************************/
/* 	File    	BSP_RTC.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 						HAl库读取RTC时 先获取时间再获取日期 顺序错误会导致读锁定
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_RTC.h"
#include "stdio.h"

RTC_HandleTypeDef hrtc;
uint8_t RTC_GetInfo(RTC_InfoTypeDef *NewInfo);
uint8_t RTC_SetInfo(RTC_InfoTypeDef *NewInfo);
RTC_InfoTypeDef DefaultInfo=
{
	.NewTime.Hours 			= 12,        	//初始值:2021/06/01/周二/12：00：00
  .NewTime.Minutes 		= 0,
	.NewTime.Seconds 		= 0,
	.NewTime.SubSeconds	= 0,
  .NewTime.DayLightSaving 	= RTC_DAYLIGHTSAVING_NONE,
  .NewTime.StoreOperation 	= RTC_STOREOPERATION_RESET,
  .NewData.Year 			= 21,
	.NewData.Month 			= RTC_MONTH_JUNE,
  .NewData.Date 			= 1,
	.NewData.WeekDay 		= 		RTC_WEEKDAY_TUESDAY,
};
/*****************************************************
	* 函数功能: RTC 时钟初始化
  * 输入参数: RTC_HandleTypeDef rtc类型
  * 返 回 值: 无
  * 说    明：无
******************************************************/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
  }
}

/*****************************************************
	* 函数功能: RTC 配置
  * 输入参数: RTC_HandleTypeDef rtc类型
  * 返 回 值: 无
  * 说    明：无
******************************************************/
void MX_RTC_Init(void)
{
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat 		= RTC_HOURFORMAT_24;							//RTC设置为24小时格式 
  hrtc.Init.AsynchPrediv 	= 127;														//异步分频系数 默认值
  hrtc.Init.SynchPrediv 	= 255;														//同步分频系数 默认值
  hrtc.Init.OutPut 				= RTC_OUTPUT_DISABLE;							//以下为闹钟配置
  hrtc.Init.OutPutRemap 	= RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity= RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType 		= RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp 	= RTC_OUTPUT_PULLUP_NONE;
  HAL_RTC_Init(&hrtc);
	if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0)!=0X5050)		//是否第一次配置
	{ 
			RTC_SetInfo(&DefaultInfo);											//配置默认值
			HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0X5050);	//标记已经初始化过了
	}
}
/*****************************************************
	* 函数功能: 获取当前芯片时间
  * 输入参数: RTC_InfoTypeDef *NewInfo
  * 返 回 值: 返回0：设置成功  1：时间设置错误 2：日期设置错误
  * 说    明：无
******************************************************/
uint8_t RTC_GetInfo(RTC_InfoTypeDef *NewInfo)
{
	if (HAL_RTC_GetTime(&hrtc,&(NewInfo->NewTime),0) != HAL_OK)
	return 1;	
	if (HAL_RTC_GetDate(&hrtc,&(NewInfo->NewData),0) != HAL_OK)
	return 2;		
	return 0;		
}
/*****************************************************
	* 函数功能: 设置当前芯片时间
  * 输入参数: RTC_InfoTypeDef *NewInfo
  * 返 回 值: 返回0：设置成功  1：时间设置错误 2：日期设置错误
  * 说    明：无
******************************************************/
uint8_t RTC_SetInfo(RTC_InfoTypeDef *NewInfo)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
	sTime.Hours 	= NewInfo->NewTime.Hours;	
  sTime.Minutes =	NewInfo->NewTime.Minutes;
  sTime.Seconds = NewInfo->NewTime.Seconds;
  sTime.SubSeconds 			= 0;
  sTime.DayLightSaving 	= RTC_DAYLIGHTSAVING_NONE;	
  sTime.StoreOperation 	= RTC_STOREOPERATION_RESET;
	if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	return 1;
 
  sDate.WeekDay = NewInfo->NewData.WeekDay;
  sDate.Month 	= NewInfo->NewData.Month;;
  sDate.Date 		= NewInfo->NewData.Date;
  sDate.Year 		= NewInfo->NewData.Year;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  return 2;
	return 0;
}
