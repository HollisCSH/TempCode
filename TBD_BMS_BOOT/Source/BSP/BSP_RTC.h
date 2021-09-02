/****************************************************************************/
/* 	File    	BSP_RTC.h 			 
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
typedef struct
{
	RTC_TimeTypeDef NewTime;
	RTC_DateTypeDef NewData;
}RTC_InfoTypeDef;

void MX_RTC_Init(void);
uint8_t RTC_GetInfo(RTC_InfoTypeDef *NewInfo);
uint8_t RTC_SetInfo(RTC_InfoTypeDef *NewInfo);
#endif /* __BSP_RTC_H__ */


