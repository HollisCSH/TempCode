/****************************************************************************/
/* 	File    	BSP_PWR.c 			 
 * 	Author		Hollis
 *	Notes			ʹ��BSPʱ���stm32g0xx_hal_conf.h��Ӧ�ĺ�
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
	* ��������: �͹���˯��ģʽ��ʼ��
  * �������: 
  * �� �� ֵ: ��
  * ˵    ����G0ϵ�е�Ƭ��Ҫ����͹���˯��ģʽ�����Ƚ�Ƶ����͹�������ģʽ���˳�������ģʽҲ��Ҫһ�������̡�
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
		MX_LPUSART1_UART_Init(115200);		//���Դ���
		//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);  
}

/*****************************************************
	* ��������: ֹͣģʽ
  * �������: 
  * �� �� ֵ: ��
  * ˵    ����G0ϵ�е�Ƭ��Ҫ����͹���˯��ģʽ�����Ƚ�Ƶ����͹�������ģʽ���˳�������ģʽҲ��Ҫһ�������̡�
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
