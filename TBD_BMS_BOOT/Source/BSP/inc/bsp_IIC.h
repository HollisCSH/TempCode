/****************************************************************************/
/* 	File    	bsp_IIC.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_IIC_H__
#define __BSP_IIC_H__

#include <stdint.h>
#include "main.h"

void MX_I2C1_Init(void);
void MX_I2C1_REAR(uint8_t *pData, uint16_t Size, uint32_t Timeout);
void MX_I2C1_WRITE(uint8_t *pData, uint16_t Size, uint32_t Timeout);
#endif /* __BSP_IIC_H__ */
