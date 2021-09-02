/****************************************************************************/
/* 	File    	BSP_RTC.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 				HAl���ȡRTCʱ �Ȼ�ȡʱ���ٻ�ȡ���� ˳�����ᵼ�¶�����
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_RTC.h"
#include "BSP_UART.h"
#include "stdio.h"

/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
static u8 gRTCTimeUp = 0;  //���ѱ���
RTC_HandleTypeDef hrtc;
/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/

/*****************************************************
	* ��������: RTC ʱ�ӡ��жϳ�ʼ��
	* �������: RTC_HandleTypeDef ����
	* �� �� ֵ: ��
	* ˵    �������ຯ���ڵ���HAL_XX_Init ������
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
	* ��������: RTC ��ʼ��
	* �������: RTC_HandleTypeDef rtc����
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void MX_RTC_Init(void)
{        
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTC_ENABLE();
    __HAL_RCC_RTCAPB_CLK_ENABLE();
	/*���ﲢ����ÿ���ϵ綼Ҫ��ʼ��RTC�����ݵ�0�����ݼĴ����������������Ϊ�ڲ��ϸ�λ����ִ�г�ʱ��RTC��
	RTC��ʱ�佫����������һ�θ�λ���������뵽һ�룬�����Ŀ��°����ҵ���ĿӦ������Ҫ��stopģʽ�»�
	�Ѳ���λ���൱�ڲ���ʱ����һ�룬�����������ҿ�����Ĺ���ʱ�䣬����HAL�ĿӰ���20201204*/
	hrtc.TampOffset = (TAMP_BASE - RTC_BASE);//һ������䣬��Ȼ�ڲ���ʼ��HAL_RTC_Init()��ʱ��ִ��HAL_RTCEx_BKUPRead(&RtcHandle,RTC_BKP_DR0)��ȡ���ɹ���Ϊ0
	hrtc.Instance    = RTC;    
	if(HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1)!=0X5050)		//�Ƿ��һ������
	{    
        
        hrtc.Init.HourFormat 	= RTC_HOURFORMAT_24;			//RTC����Ϊ24Сʱ��ʽ 
        hrtc.Init.AsynchPrediv 	= 127;							//�첽��Ƶϵ�� Ĭ��ֵ
        hrtc.Init.SynchPrediv 	= 255;							//ͬ����Ƶϵ�� Ĭ��ֵ
        hrtc.Init.OutPut 		= RTC_OUTPUT_DISABLE;	//����Ϊ��������
        hrtc.Init.OutPutRemap 	= RTC_OUTPUT_REMAP_NONE;
        hrtc.Init.OutPutPolarity= RTC_OUTPUT_POLARITY_HIGH;
        hrtc.Init.OutPutType 	= RTC_OUTPUT_TYPE_OPENDRAIN;
        hrtc.Init.OutPutPullUp 	= RTC_OUTPUT_PULLUP_NONE;  
        HAL_RTC_Init(&hrtc);
		RTC_SetInfo(&DefaultInfo);							//����Ĭ��ֵ
        
		HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR1,0X5050);		//����Ѿ���ʼ������
        HAL_RTCEx_BKUPRead(&hrtc,RTC_BKP_DR1);
	}
}

/*****************************************************
	* ��������: RTC ��ʼ��
	* �������: RTC_HandleTypeDef rtc����
	* �� �� ֵ: ��
	* ˵    ���������ⲿ����
******************************************************/
void BSPRTCInit(void)
{
	MX_RTC_Init();
}

/*****************************************************
	* ��������: ��ȡ��ǰоƬʱ��
	* �������: RTC_InfoTypeDef *NewInfo
	* �� �� ֵ: ����0����ȡ�ɹ�  1��ʱ���ȡ���� 2�����ڻ�ȡ����
	* ˵    ������
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
	* ��������: ���õ�ǰоƬʱ��
	* �������: RTC_InfoTypeDef *NewInfo
	* �� �� ֵ: ����0�����óɹ�  1��ʱ�����ô��� 2���������ô���
	* ˵    �������ڼ���ʹ�ã��������Բ�����ֵ
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
//��������	: void BSPRTCSetAlarmAfterSeconds(u32 sec)
//�������	: sec:��	
//�������	: void
//��̬����	: 
//��    ��	: ����RTC��sec������RTC�ж�
//ע    ��	: ����������������A/B,�˴���ʱ�Ϊ65536�� BCD�����ӷ�������ת��
//=============================================================================================
void BSPRTCSetAlarmAfterSeconds(uint16_t sec)
{
	t_PCF85063_DATE pRdTimeAndDate;
	uint8_t pHours,pMinutes,pSeconds;
	RTC_AlarmTypeDef sAlarm = {0};
	RTC_GetInfo(&pRdTimeAndDate);
	pHours    = sec/3600;         //��ȡ���ӵ�ʱ����
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

	sAlarm.Alarm = RTC_ALARM_A;                                     //A �¼���ӦA�ص�����
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;   //�������ڻ������ڵ�ѡ��
	sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;                   //��������ڻ������ڼ�
	sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY ;                  //ʱ����ƥ�伴�����ж�
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;        //��������������
	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);            //���������ж�
}

//=============================================================================================
//��������	: void BSPRTCStop(void)
//�������	: void
//�������	: void
//��̬����	:
//��    ��	: RTCֹͣ���� 
//ע    ��	:
//=============================================================================================
void BSPRTCStop(void)
{
    HAL_RTC_DeactivateAlarm(&hrtc , RTC_ALARM_A);
}

//=============================================================================================
//��������	: void BSPRTCGetTimeUp(void)
//�������	: void
//�������	: 1����ʱ������0����ʱδ��
//��̬����	:
//��    ��	: ��ȡRTC��ʱ��־
//ע    ��	:
//=============================================================================================
u8 BSPRTCGetTimeUp(void)
{
    return gRTCTimeUp;
}

//=============================================================================================
//��������	: void BSPRTCClrTimeUp(void)
//�������	: void
//�������	: void
//��̬����	:
//��    ��	: ���RTC��ʱ��־
//ע    ��	:
//=============================================================================================
void BSPRTCClrTimeUp(void)
{
    gRTCTimeUp = 0;
}
 
//=============================================================================================
//��������	: void HAL_RTC_AlarmAEventCallback
//�������	: void
//�������	: void
//��̬����	:
//��    ��	: RTC Alarm�жϴ�����
//ע    ��	:
//=============================================================================================
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	/*����MCU�����ڻ��� */
	gRTCTimeUp = 1;
	_UN_NB_Printf("RTC Alarm A\n");
}




/*****************************************************
	* ��������: ��ȡʱ�����
	* �������: void
	* �� �� ֵ: ��
	* ˵    ������
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


