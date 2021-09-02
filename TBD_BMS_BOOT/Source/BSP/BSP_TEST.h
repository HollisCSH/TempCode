/****************************************************************************/
/* 	File    	BSP_TEST.h 			 
 * 	Author		Hollis
 *	Notes
 * 
 *	Version		Date    		Change Logs:
 *  V1.0.0 		2021-06-025	first version         	
 * 
 * Copyright (c) 2021, Immotor
*****************************************************************************/

#ifndef __BSP_TEST_H__
#define __BSP_TEST_H__

#include <stdint.h>
#include "main.h"

void MX_RTC_Test(void);
void MX25L16ReadDevID(void);
void MX_CAN_TEST(void);
void MX_IIC_AFE_TEST(void);
void MX_IIC_ACC_TEST(void);

#endif /* __BSP_TEST_H__ */
