/****************************************************************************/
/* 	File    	bsp_SPI.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_SPI_H__
#define __BSP_SPI_H__

#include <stdint.h>
#include "main.h"
void MX_SPI3_Init(void);
uint8_t SPI_Transmit(uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint8_t SPI_Receive(uint8_t *pData, uint16_t Size, uint32_t Timeout);

#endif /* __BSP_SPI_H__ */
