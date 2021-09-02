/****************************************************************************/
/* 	File    	BSP_PWR.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_PWR.h"
#include "bsp_SYSCLK.h"
#include "bsp_GPIO.h"
#include "bsp_UART.h"

/*****************************************************
	* 函数功能: 低功耗睡眠模式初始化
  * 输入参数: 
  * 返 回 值: 无
  * 说    明：G0系列单片机要进入低功耗睡眠模式，需先降频进入低功耗运行模式，退出至正常模式也需要一样的流程。
******************************************************/
void MCU_INTO_SLEEP_MODE(void)
{		
		__HAL_RCC_PWR_CLK_ENABLE();
		SystemClock_Decrease();
		HAL_SuspendTick();
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5);
		/* Enter Sleep Mode, wake up is done once User push-button is pressed */
		HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
		/* System is Low Power Run mode when exiting Low Power Sleep mode,
		disable low power run mode and reset the clock to initialization configuration */ 
		HAL_PWREx_DisableLowPowerRunMode();
		/* Configure the system clock for the RUN mode */
		SystemClock_Config();
		MX_GPIO_Init();
		MX_LPUSART1_UART_Init(115200);		//调试串口
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  
}

/*****************************************************
	* 函数功能: 停止模式
  * 输入参数: 
  * 返 回 值: 无
  * 说    明：G0系列单片机要进入低功耗睡眠模式，需先降频进入低功耗运行模式，退出至正常模式也需要一样的流程。
******************************************************/
void MCU_INTO_STOP_MODE(void)	
{
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void BSPLowPowInit(void)
{
//	_Debug_Printf("begin sleep mode\r\n");
	HAL_SuspendTick();
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON,PWR_SLEEPENTRY_WFI);
	HAL_ResumeTick();
}
