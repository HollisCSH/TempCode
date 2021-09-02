/****************************************************************************/
/* 	File    	bsp_SYSCLK.h 			 
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

#endif /* __BSP_SYSCLK_H__ */
