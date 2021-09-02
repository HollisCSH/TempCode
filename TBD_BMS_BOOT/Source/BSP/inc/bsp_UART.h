/****************************************************************************/
/* 	File    	bsp_UART.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_UART_H__
#define __BSP_UART_H__
#include <stdint.h>
#include <stdio.h>
#include "main.h"
void HAL_UART_MspInit(UART_HandleTypeDef* huart);
void MX_USART1_UART_Init(uint32_t baud);
void MX_LPUSART1_UART_Init(uint32_t baud);

#endif /* __BSP_UART_H__ */
