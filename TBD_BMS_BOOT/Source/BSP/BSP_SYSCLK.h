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

void SystemClock_Config(void);              //系统时钟配置

void INT_SYS_DisableIRQGlobal(void);        //关中断

void INT_SYS_EnableIRQGlobal(void);         //关中断
#endif /* __BSP_SYSCLK_H__ */
