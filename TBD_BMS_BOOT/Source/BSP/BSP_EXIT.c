/****************************************************************************/
/* 	File    	BSP_EXIT.c 			 
 * 	Author		Hollis
 *	Notes		使用BSP时需打开stm32g0xx_hal_conf.h对应的宏
 * 				用外部中断时  需要注意中断触发方式  原始电平  与其他引脚复用是否有冲突
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	    first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_EXIT.h"

/* 私有宏定义 ----------------------------------------------------------------*/
static uint8_t g_ID_flag = 0;	//外置模块接入标志   接入后产生中断置1

/********************************************************************
	* 函数功能: IO中断初始化
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：中断对应的函数入口位于 HAL_GPIO_EXTI_Falling_Callback
********************************************************************/
void BSPExitInit()
{
	GPIO_InitTypeDef gpioinitstruct;
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	gpioinitstruct.Pin = MODULE_INT_PIN;
	gpioinitstruct.Pull = GPIO_NOPULL;
	gpioinitstruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	gpioinitstruct.Mode = GPIO_MODE_IT_FALLING;		//下降沿触发  GPIO_MODE_IT_FALLING  GPIO_MODE_IT_RISING
	HAL_GPIO_Init(MODULE_INT_PORT, &gpioinitstruct);
	
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0x03, 0x00);
//	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);		//睡眠前开启  暂时关闭
}

//=============================================================================================
//函数名称	: void HAL_GPIO_EXTI_Falling_Callback(void)
//函数参数	: 具体中断线
//输出参数	: void
//静态变量	: void
//功    能	: 下降沿中断
//注    意	: 系统函数 由 stm32g0xx_it.c 中的中断函数引入
//=============================================================================================
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MODULE_INT_PIN)
    {
        g_ID_flag = 1;
    }
}

/********************************************************************
	* 函数功能: 设置 外置设备接入 状态
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：
********************************************************************/
void SET_MODU_STA(uint8_t flag)
{
	g_ID_flag = flag;
}

/********************************************************************
	* 函数功能: 获取 外置设备接入 状态
	* 输入参数: 无
	* 返 回 值: 无
	* 说    明：
********************************************************************/
uint8_t GET_MODU_INT_STA(void)
{
	return g_ID_flag;
}
