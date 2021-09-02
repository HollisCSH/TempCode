/****************************************************************************/
/* 	File    	BSP_EXIT.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 				使用外部中断时  需要注意中断触发方式  原始电平  与其他引脚复用是否有冲突
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_EXIT.h"
#include "BSP_GPIO.h"
#include "IOCheck.h"

/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/

/*****************************************************
	* 函数功能: IO中断初始化
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：中断对应的函数入口位于外部
******************************************************/
void BSP_EXIT_Init()
{
	GPIO_InitTypeDef gpioinitstruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	/**************************************************/
	gpioinitstruct.Pin = ALARM_AFE_PIN|REMOVE_INT_PIN|MODULE_INT_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;		//下降沿触发	
	HAL_GPIO_Init(GPIOA, &gpioinitstruct);
	
//	gpioinitstruct.Pin = MODULE_SHORT_PIN;	//外置模块接入中断
//	gpioinitstruct.Pull = GPIO_NOPULL;
//	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//	gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//上升沿触发	
//	HAL_GPIO_Init(GPIOA, &gpioinitstruct);	
	/**************************************************/
	gpioinitstruct.Pin = ACC_INT1_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//上升沿触发
	HAL_GPIO_Init(GPIOB, &gpioinitstruct);
	/**************************************************/
	gpioinitstruct.Pin = INT_4G_PIN ;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;		//下降沿触发
	HAL_GPIO_Init(GPIOC, &gpioinitstruct);
	
	gpioinitstruct.Pin = ACC_INT2_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_RISING;		//上升沿触发
	HAL_GPIO_Init(ACC_INT2_PORT, &gpioinitstruct);
	/**************************************************/
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0x03, 0);
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0x03, 0);
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x03, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
}
