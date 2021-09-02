/****************************************************************************/
/* 	File    	BSP_PWR.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_PWR_H__
#define __BSP_PWR_H__

#include <stdint.h>
#include "main.h"
void MCU_INTO_SLEEP_MODE(void);
void MCU_INTO_STOP_MODE(void);
void BSPLowPowInit(void);

#endif /* __BSP_PWR_H__ */
