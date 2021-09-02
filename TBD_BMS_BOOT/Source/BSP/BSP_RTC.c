/****************************************************************************/
/* 	File    	BSP_RTC.c 			 
 * 	Author		Hollis
 *	Notes			ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 						HAl���ȡRTCʱ �Ȼ�ȡʱ���ٻ�ȡ���� ˳�����ᵼ�¶�����
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
	.NewTime.Hours 			= 12,        	//��ʼֵ:2021/06/01/�ܶ�/12��00��00
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
	* ��������: RTC ʱ�ӳ�ʼ��
  * �������: RTC_HandleTypeDef rtc����
  * �� �� ֵ: ��
  * ˵    ������
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
	* ��������: RTC ����
  * �������: RTC_HandleTypeDef rtc����
  * �� �� ֵ: ��
  * ˵    ������
******************************************************/
void MX_RTC_Init(void)
{
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat 		= RTC_HOURFORMAT_24;							//RTC����Ϊ24Сʱ��ʽ 
  hrtc.Init.AsynchPrediv 	= 127;														//�첽��Ƶϵ�� Ĭ��ֵ
  hrtc.Init.SynchPrediv 	= 255;														//ͬ����Ƶϵ�� Ĭ��ֵ
  hrtc.Init.OutPut 				= RTC_OUTPUT_DISABLE;							//����Ϊ��������
  hrtc.Init.OutPutRemap 	= RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity= RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType 		= RTC_OUTPUT_TYPE_OPENDRAIN;
  hrtc.Init.OutPutPullUp 	= RTC_OUTPUT_PULLUP_NONE;
  HAL_RTC_Init(&hrtc);
	if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR0)!=0X5050)		//�Ƿ��һ������
	{ 
			RTC_SetInfo(&DefaultInfo);											//����Ĭ��ֵ
			HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0X5050);	//����Ѿ���ʼ������
	}
}
/*****************************************************
	* ��������: ��ȡ��ǰоƬʱ��
  * �������: RTC_InfoTypeDef *NewInfo
  * �� �� ֵ: ����0�����óɹ�  1��ʱ�����ô��� 2���������ô���
  * ˵    ������
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
	* ��������: ���õ�ǰоƬʱ��
  * �������: RTC_InfoTypeDef *NewInfo
  * �� �� ֵ: ����0�����óɹ�  1��ʱ�����ô��� 2���������ô���
  * ˵    ������
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
