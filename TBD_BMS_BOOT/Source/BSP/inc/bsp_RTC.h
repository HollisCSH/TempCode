/****************************************************************************/
/* 	File    	bsp_RTC.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_RTC_H__
#define __BSP_RTC_H__

#include <stdint.h>
#include "main.h"
void MX_RTC_Init(void);
void RTC_GetTime(RTC_TimeTypeDef *NewTime);
void RTC_GetDate(RTC_DateTypeDef *NewData);

#endif /* __BSP_RTC_H__ */


