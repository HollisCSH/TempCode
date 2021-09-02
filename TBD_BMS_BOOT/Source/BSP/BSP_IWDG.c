/****************************************************************************/
/* 	File    	BSP_IWDG.c 			 
 * 	Author		Hollis
 *	Notes			使用BSP时需打开stm32g0xx_hal_conf.h对应的宏,使用DEBUG模式下不使用看门狗
 * 				
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 	
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#include "BSP_IWDG.h"
IWDG_HandleTypeDef hiwdg;

/**
  * 函数功能: 独立看门狗初始化配置	(（可在待机和停止模式下运行）)
  * 输入参数: prv可以是[4,8,16,32,64,128,256]
  *            prv:预分频器值，取值如下：
  *            参数 IWDG_PRESCALER_4: IWDG prescaler set to 4
  *            参数 IWDG_PRESCALER_8: IWDG prescaler set to 8
  *            参数 IWDG_PRESCALER_16: IWDG prescaler set to 16
  *            参数 IWDG_PRESCALER_32: IWDG prescaler set to 32
  *            参数 IWDG_PRESCALER_64: IWDG prescaler set to 64
  *            参数 IWDG_PRESCALER_128: IWDG prescaler set to 128
  *            参数 IWDG_PRESCALER_256: IWDG prescaler set to 256
  *
  *            rlv:预分频器值，取值范围为：0-0XFFF
  * 返 回 值: 无
  * 说    明：函数调用举例：Counter Reload Value = (LsiFreq(Hz) * Timeout(ms)) / (prescaler * 1000)   
  *           IWDG_Config(IWDG_Prescaler_32 ,1000);  // IWDG 1s 超时溢出 =(32000 * 1000) / (32 * 1000)
  *			  时钟非标准32K  所以计算值只是大约值
  */
  
//#define DEBUG
/*****************************************************
	* 函数功能: 看门狗初始化
	* 输入参数: rlv Min_Data = 0 and Max_Data = 0x0FFF
	* 返 回 值: void
	* 说    明：开启后无法关断开门狗，可以冻结
******************************************************/
void MX_IWDG_Init(uint32_t rlv)
{
	#ifndef DEBUG
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32;//设置IWDG分频系数
    hiwdg.Init.Reload = rlv;                 //重装载 
    hiwdg.Init.Window = IWDG_WINDOW_DISABLE; //关闭窗口功能

    if(HAL_IWDG_Init(&hiwdg) != HAL_OK)
    {
        assert_param(0);
    }
    #endif
}	

/*****************************************************
	* 函数功能: 喂狗函数
	* 输入参数: void
	* 返 回 值: void
	* 说    明：
******************************************************/
void  FeedIDog(void)
{
    #ifndef DEBUG
    if(HAL_IWDG_Refresh(&hiwdg) != HAL_OK)
    {
        assert_param(0);
    }
    #endif
}
