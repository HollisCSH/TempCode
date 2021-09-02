/****************************************************************************/
/* 	File    	BSP_TIM.c 			 
 * 	Author		Hollis
 *	Notes		ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_TIM.h"
#include "stdio.h"

/* ˽�к궨�� ----------------------------------------------------------------*/
#define FeedDogTime 3               //���ߺ�3������ι��
/* ˽�б��� ------------------------------------------------------------------*/
TIM_HandleTypeDef	htim2;
TIM_HandleTypeDef	htim3;
LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim2;
static uint8_t gTimFeedDog = 0;     //�͹��Ķ�ʱ������ι����־ =1����ι��
/* ��չ���� ------------------------------------------------------------------*/
/* �������� ------------------------------------------------------------------*/

/*****************************************************
	* ��������: LPTIMʱ�ӡ��жϳ�ʼ��
	* �������: LPTIM_HandleTypeDef hlptim����
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim)
{
	if(hlptim->Instance==LPTIM1)
	{
		__HAL_RCC_LPTIM1_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM6_DAC_LPTIM1_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM6_DAC_LPTIM1_IRQn);
	}
	else if(hlptim->Instance==LPTIM2)
	{
		__HAL_RCC_LPTIM2_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM7_LPTIM2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM7_LPTIM2_IRQn);
	}
}
/*****************************************************
	* ��������: LPTIMʱ�ӡ��жϹض�
	* �������: LPTIM_HandleTypeDef hlptim����
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim)
{
	if(hlptim->Instance==LPTIM1)
	{
		__HAL_RCC_LPTIM1_CLK_DISABLE();
		HAL_NVIC_DisableIRQ(TIM6_DAC_LPTIM1_IRQn);
	}
	else if(hlptim->Instance==LPTIM2)
	{
		__HAL_RCC_LPTIM2_CLK_DISABLE();
		HAL_NVIC_DisableIRQ(TIM7_LPTIM2_IRQn);
	}
}

/*****************************************************
	* ��������: TIMʱ�ӡ��жϳ�ʼ��
	* �������: TIM_HandleTypeDef 
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim)
{
	if(htim->Instance==TIM2)
	{
		__HAL_RCC_TIM2_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
	}
	if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();
		HAL_NVIC_SetPriority(TIM3_TIM4_IRQn, 3, 0);
		HAL_NVIC_EnableIRQ(TIM3_TIM4_IRQn);
	}
}

/*****************************************************
	* ��������: TIMʱ�ӡ��ж� �ض�
	* �������: TIM_HandleTypeDef 
	* �� �� ֵ: ��
	* ˵    ������
******************************************************/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim)
{
	if(htim->Instance==TIM2)
	{
		__HAL_RCC_TIM2_CLK_DISABLE();
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
	}
	if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_DISABLE();
		HAL_NVIC_DisableIRQ(TIM3_TIM4_IRQn);
	}
}
/*****************************************************
	* ��������: TIM2��������
	* �������: void
	* �� �� ֵ: uint8_t 0 Ϊ���óɹ�	��0��ʾ����ʧ��
	* ˵    ������
******************************************************/
uint8_t MX_TIM2_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 15;				//PRESCALER_VALUE	15999
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;	
	htim2.Init.Period = 999 ;				//PERIOD_VALUE		1000-1
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.RepetitionCounter = 0;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		return 1;
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		return 2;
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		return 3;
	}
	HAL_TIM_Base_Start_IT(&htim2);
	return 0;
}
/*****************************************************
	* ��������: TIM3��������
	* �������: void
	* �� �� ֵ: uint8_t 0 Ϊ���óɹ�	��0��ʾ����ʧ��
	* ˵    ������
******************************************************/
uint8_t MX_TIM3_Init(void)	//��Ϊ�ĵ��ӳٺ���
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 15;		//��Ƶ��1M
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 999;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.RepetitionCounter = 0;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
	{
		return 1;
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
	{
		return 2;
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
	{
		return 3;
	}
	//	HAL_TIM_Base_Start_IT(&htim3);	
	return 0;
}
/*****************************************************
	* ��������: LPTIM��������
	* �������: void
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    �����ö�ʱ���������߶�ʱ����ι��
******************************************************/
uint8_t MX_LPTIM1_Init(void)
{
	hlptim1.Instance = LPTIM1;
	hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;  	//ѡ���ڲ�ʱ��Դ
	hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV2;          	//����LPTIMʱ�ӷ�Ƶ
	hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;                        //�����������
	hlptim1.Init.Trigger.ActiveEdge = LPTIM_ACTIVEEDGE_RISING;                      //���������ش���
	hlptim1.Init.Trigger.SampleTime = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;        //����ʱ�Ӹ����˲���
	hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;     	//��������ߵ�ƽ
	hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;            	//�ȽϼĴ�����ARR�Զ����ؼĴ���ѡ����ĺ���������
	hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;   	//LPTIM���������ڲ�ʱ��Դ����
	hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;         	//�ⲿ����1��������δʹ��
	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;         	//�ⲿ����2��������δʹ��
	if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
	{
		return 1;
	}
	if (HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 0, 49151) != HAL_OK)   //1��Ƶ ��ʱ��32768Ϊ1�� �ô�����2��Ƶ 49152 Ϊ3��
	{
		return 2;
	}
	return 0;
}
/*****************************************************
	* ��������: LPTIM�ر�
	* �������: void
	* �� �� ֵ: ��
	* ˵    ����
******************************************************/
void MX_LPTIM1_DeInit(void)
{
    HAL_LPTIM_DeInit(&hlptim1);
}
/*****************************************************
	* ��������: LPTIM��������
	* �������: void
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    ������
******************************************************/
uint8_t MX_LPTIM2_Init(void)
{
	hlptim2.Instance = LPTIM2;
	hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;  	//ѡ���ڲ�ʱ��Դ
	hlptim2.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;           	//����LPTIMʱ�ӷ�Ƶ
	hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;                        //�����������
	hlptim2.Init.Trigger.ActiveEdge = LPTIM_ACTIVEEDGE_RISING;                      //���������ش���
	hlptim2.Init.Trigger.SampleTime = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;        //����ʱ�Ӹ����˲���
	hlptim2.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;     	//��������ߵ�ƽ
	hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;            	//�ȽϼĴ�����ARR�Զ����ؼĴ���ѡ����ĺ���������
	hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;   	//LPTIM���������ڲ�ʱ��Դ����
	hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;         	//�ⲿ����1��������δʹ��
	hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;         	//�ⲿ����2��������δʹ��
	if (HAL_LPTIM_Init(&hlptim2) != HAL_OK)
	{
		return 1;
	}
	if (HAL_LPTIM_TimeOut_Start_IT(&hlptim2, 0, 32767) != HAL_OK)
	{
		return 2;
	}
	return 0;
}

/*****************************************************
	* ��������: ��ʱ����ʼ��
	* �������: void
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    ������
******************************************************/
void BSPPITInit(void)
{
	MX_TIM2_Init();
	MX_TIM3_Init();
//	MX_LPTIM1_Init();
//	MX_LPTIM2_Init();
}

/*****************************************************
	* ��������: ��ʱ��ֹͣ
	* �������: void
	* �� �� ֵ: uint8_t 0 Ϊ���ͳɹ�	��0��ʾ����ʧ��
	* ˵    ������
******************************************************/
void BSPTimerStop(void)
{
	HAL_TIM_Base_DeInit(&htim2);
	HAL_TIM_Base_DeInit(&htim3);
}


/*****************************************************
	* ��������: us�ӳ�
	* �������: us
	* �� �� ֵ: ��
	* ˵    ����������
******************************************************/
void delay_xus(uint16_t us)
{
	volatile uint16_t differ=0xffff-us-5;		//�趨��ʱ����������ʼֵ
	__HAL_TIM_SET_COUNTER(&htim3,differ);
	HAL_TIM_Base_Start(&htim3);					//������ʱ��
	while(differ<0xffff-6)						//�������ж�
	{
		differ=__HAL_TIM_GET_COUNTER(&htim3);	//��ѯ�������ļ���ֵ
	}
	HAL_TIM_Base_Stop(&htim3);
}

/*****************************************************
	* ��������: LPTIM_�жϻص�����  ����
	* �������: LPTIM_HandleTypeDef
	* �� �� ֵ: ��
	* ˵    ����ϵͳ���� ��stm32g0xx_it.c���жϺ�������
******************************************************/
void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
	if(hlptim->Instance==LPTIM1)
	{
        gTimFeedDog = 1;
//		printf("LPTIM1 %d \r\n",HAL_GetTick());       
	} 
	else if(hlptim->Instance==LPTIM2)
	{	
	}
}
/*****************************************************
	* ��������: ��ȡ�͹��Ķ�ʱ��ι����־
	* �������: ��
	* �� �� ֵ: �� 
	* ˵    ����
******************************************************/
uint8_t Get_TIM_FeedDogFlag(void)
{
    return gTimFeedDog;
}

/*****************************************************
	* ��������: ����͹��Ķ�ʱ��ι����־
	* �������: ��
	* �� �� ֵ: ��
	* ˵    ����
******************************************************/
void BSP_TIM_ClearFeedDogFlag(void)
{
    gTimFeedDog = 0;
}

/*****************************************************
	* ��������: TIM_�жϻص�����  ����
	* �������: TIM_HandleTypeDef
	* �� �� ֵ: ��
	* ˵    ����ϵͳ���� ��stm32g0xx_it.c���жϺ�������
******************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance==TIM2)
	{
	} 
	if(htim->Instance==TIM3)
	{

	}
}


