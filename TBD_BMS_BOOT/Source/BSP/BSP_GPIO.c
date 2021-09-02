/****************************************************************************/
/* 	File    	BSP_GPIO.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_GPIO.h"
#include "BSP_EXIT.h"

/*****************************************************
	* 函数功能: GPIO端口 时钟 中断初始化
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void BSPGPIOInit(void)
{
	GPIO_InitTypeDef  gpioinitstruct;
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	gpioinitstruct.Pin = PWR_EN_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(PWR_EN_PORT, &gpioinitstruct);
	
	gpioinitstruct.Pin = CAN_5V_PIN;
	gpioinitstruct.Mode = GPIO_MODE_OUTPUT_PP;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(CAN_5V_PORT, &gpioinitstruct);
	
    BSPExitInit();									//外部中断	
}

/*****************************************************
	* 函数功能: GPIO端口 关断
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：无
******************************************************/
void BSPGPIODeInit(void)
{
    HAL_GPIO_DeInit(PWR_EN_PORT , PWR_EN_PIN);
    HAL_GPIO_DeInit(CAN_5V_PORT , CAN_5V_PIN);
}

/*****************************************************
	* 函数功能: 使能CAN通讯芯片 供电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void CAN_POWER_ENABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* 函数功能: 禁止CAN通讯芯片 断电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void CAN_POWER_DISABLE(void)
{
	HAL_GPIO_WritePin(CAN_5V_PORT, CAN_5V_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_RESET);
}

/*****************************************************
	* 函数功能: 使能14V电源输出
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void PWR_EN_ENABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* 函数功能: 禁止14V电源输出
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void PWR_EN_DISABLE(void)
{
	HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_RESET);
}
/*****************************************************
	* 函数功能: 使能FLASH通讯芯片 供电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_POWER_ENABLE(void)
{
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_RESET);	//低电平打开通电
}

/*****************************************************
	* 函数功能: 禁止FLASH通讯芯片 断电
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_POWER_DISABLE(void)
{
	HAL_GPIO_WritePin(FLASH_POW_EN_PORT, FLASH_POW_EN_PIN, GPIO_PIN_SET);
}

/*****************************************************
	* 函数功能: 使能FLASH芯片 通讯
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_CS_ENABLE(void)
{
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);
}

/*****************************************************
	* 函数功能: 禁止FLASH芯片 通讯
    * 输入参数: 无
    * 返 回 值: 无
    * 说    明：无
******************************************************/
void FLASH_CS_DISABLE(void)
{
	HAL_GPIO_WritePin(FLASH_CS_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

