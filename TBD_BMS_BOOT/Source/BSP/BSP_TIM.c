/****************************************************************************/
/* 	File    	BSP_TIM.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08		first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_TIM.h"
#include "stdio.h"

/* 私有宏定义 ----------------------------------------------------------------*/
#define FeedDogTime 3               //休眠后3秒起唤醒喂狗
/* 私有变量 ------------------------------------------------------------------*/
TIM_HandleTypeDef	htim2;
TIM_HandleTypeDef	htim3;
LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim2;
static uint8_t gTimFeedDog = 0;     //低功耗定时器唤醒喂狗标志 =1可以喂狗
/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/

/*****************************************************
	* 函数功能: LPTIM时钟、中断初始化
	* 输入参数: LPTIM_HandleTypeDef hlptim类型
	* 返 回 值: 无
	* 说    明：无
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
	* 函数功能: LPTIM时钟、中断关断
	* 输入参数: LPTIM_HandleTypeDef hlptim类型
	* 返 回 值: 无
	* 说    明：无
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
	* 函数功能: TIM时钟、中断初始化
	* 输入参数: TIM_HandleTypeDef 
	* 返 回 值: 无
	* 说    明：无
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
	* 函数功能: TIM时钟、中断 关断
	* 输入参数: TIM_HandleTypeDef 
	* 返 回 值: 无
	* 说    明：无
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
	* 函数功能: TIM2基础配置
	* 输入参数: void
	* 返 回 值: uint8_t 0 为配置成功	非0表示配置失败
	* 说    明：无
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
	* 函数功能: TIM3基础配置
	* 输入参数: void
	* 返 回 值: uint8_t 0 为配置成功	非0表示配置失败
	* 说    明：无
******************************************************/
uint8_t MX_TIM3_Init(void)	//作为四等延迟函数
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 15;		//分频至1M
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
	* 函数功能: LPTIM基础配置
	* 输入参数: void
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：该定时器用于休眠定时唤醒喂狗
******************************************************/
uint8_t MX_LPTIM1_Init(void)
{
	hlptim1.Instance = LPTIM1;
	hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;  	//选择内部时钟源
	hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV2;          	//设置LPTIM时钟分频
	hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;                        //设置软件触发
	hlptim1.Init.Trigger.ActiveEdge = LPTIM_ACTIVEEDGE_RISING;                      //设置上升沿触发
	hlptim1.Init.Trigger.SampleTime = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;        //设置时钟干扰滤波器
	hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;     	//设置输出高电平
	hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;            	//比较寄存器和ARR自动重载寄存器选择更改后立即更新
	hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;   	//LPTIM计数器对内部时钟源计数
	hlptim1.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;         	//外部输入1，本配置未使用
	hlptim1.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;         	//外部输入2，本配置未使用
	if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
	{
		return 1;
	}
	if (HAL_LPTIM_TimeOut_Start_IT(&hlptim1, 0, 49151) != HAL_OK)   //1分频 计时到32768为1秒 该处设置2分频 49152 为3秒
	{
		return 2;
	}
	return 0;
}
/*****************************************************
	* 函数功能: LPTIM关闭
	* 输入参数: void
	* 返 回 值: 无
	* 说    明：
******************************************************/
void MX_LPTIM1_DeInit(void)
{
    HAL_LPTIM_DeInit(&hlptim1);
}
/*****************************************************
	* 函数功能: LPTIM基础配置
	* 输入参数: void
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：无
******************************************************/
uint8_t MX_LPTIM2_Init(void)
{
	hlptim2.Instance = LPTIM2;
	hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;  	//选择内部时钟源
	hlptim2.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;           	//设置LPTIM时钟分频
	hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;                        //设置软件触发
	hlptim2.Init.Trigger.ActiveEdge = LPTIM_ACTIVEEDGE_RISING;                      //设置上升沿触发
	hlptim2.Init.Trigger.SampleTime = LPTIM_TRIGSAMPLETIME_DIRECTTRANSITION;        //设置时钟干扰滤波器
	hlptim2.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;     	//设置输出高电平
	hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;            	//比较寄存器和ARR自动重载寄存器选择更改后立即更新
	hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;   	//LPTIM计数器对内部时钟源计数
	hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;         	//外部输入1，本配置未使用
	hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;         	//外部输入2，本配置未使用
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
	* 函数功能: 定时器初始化
	* 输入参数: void
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：无
******************************************************/
void BSPPITInit(void)
{
	MX_TIM2_Init();
	MX_TIM3_Init();
//	MX_LPTIM1_Init();
//	MX_LPTIM2_Init();
}

/*****************************************************
	* 函数功能: 定时器停止
	* 输入参数: void
	* 返 回 值: uint8_t 0 为发送成功	非0表示发送失败
	* 说    明：无
******************************************************/
void BSPTimerStop(void)
{
	HAL_TIM_Base_DeInit(&htim2);
	HAL_TIM_Base_DeInit(&htim3);
}


/*****************************************************
	* 函数功能: us延迟
	* 输入参数: us
	* 返 回 值: 无
	* 说    明：死等型
******************************************************/
void delay_xus(uint16_t us)
{
	volatile uint16_t differ=0xffff-us-5;		//设定定时器计数器起始值
	__HAL_TIM_SET_COUNTER(&htim3,differ);
	HAL_TIM_Base_Start(&htim3);					//启动定时器
	while(differ<0xffff-6)						//补偿，判断
	{
		differ=__HAL_TIM_GET_COUNTER(&htim3);	//查询计数器的计数值
	}
	HAL_TIM_Base_Stop(&htim3);
}

/*****************************************************
	* 函数功能: LPTIM_中断回调函数  处理
	* 输入参数: LPTIM_HandleTypeDef
	* 返 回 值: 无
	* 说    明：系统函数 由stm32g0xx_it.c的中断函数引入
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
	* 函数功能: 获取低功耗定时器喂狗标志
	* 输入参数: 无
	* 返 回 值: 无 
	* 说    明：
******************************************************/
uint8_t Get_TIM_FeedDogFlag(void)
{
    return gTimFeedDog;
}

/*****************************************************
	* 函数功能: 清除低功耗定时器喂狗标志
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：
******************************************************/
void BSP_TIM_ClearFeedDogFlag(void)
{
    gTimFeedDog = 0;
}

/*****************************************************
	* 函数功能: TIM_中断回调函数  处理
	* 输入参数: TIM_HandleTypeDef
	* 返 回 值: 无
	* 说    明：系统函数 由stm32g0xx_it.c的中断函数引入
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


