/****************************************************************************/
/* 	File    	BSP_SYSCLK.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_SYSCLK_H__
#define __BSP_SYSCLK_H__

#include <stdint.h>
#include "main.h"
void SystemClock_Decrease(void);
void SystemClock_Config(void);

/*****************************************************
	* 函数功能: 系统初始化
	* 输入参数: void
	* 返 回 值: 
	* 说    明：针对睡眠后唤醒 HAL库和时钟初始化
******************************************************/
void BSPICSInit(void);									//HAL_NVIC_SystemReset(); 系統復位	

//=============================================================================================
//函数名称	: void BSPChangeInterruptPrio(void)
//输入参数	: void
//输出参数	: void
//静态变量	: void
//功    能	: 改变中断优先级 
//注    意	:  
//=============================================================================================
void BSPChangeInterruptPrio(void);

void INT_SYS_DisableIRQGlobal(void);    //关中断

void INT_SYS_EnableIRQGlobal(void);   	//关中断

#endif /* __BSP_SYSCLK_H__ */
