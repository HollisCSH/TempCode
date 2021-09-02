/****************************************************************************/
/* 	File    	BSP_ADC.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include <stdint.h>
#include "main.h"

void MX_ADC1_Init(void);
uint32_t ADC_Get_Average(uint8_t ch,uint8_t times);
void MX_ADC_TEST(void);
#endif /* __BSP_ADC_H__ */
