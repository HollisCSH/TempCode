/****************************************************************************/
/* 	File    	bsp_CAN.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-08	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/
#ifndef __BSP_CAN_H__
#define __BSP_CAN_H__

#include <stdint.h>

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);
uint8_t FDCAN1_SendMsg( uint32_t exfid ,unsigned char * pbuff , unsigned short len );
uint8_t FDCAN2_SendMsg( uint32_t exfid ,unsigned char * pbuff , unsigned short len );

#endif /* __BSP_CAN_H__ */

